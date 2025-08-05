#include "ImageDiff.hpp"

ImageDiff::ImageDiff(): _minX(0), _minY(0), _maxX(0), _maxY(0)
{

}

int	ImageDiff::getMinX() const
{
	return (_minX);
}

int	ImageDiff::getMinY() const
{
	return (_minY);
}

int	ImageDiff::getMaxX() const
{
	return (_maxX);
}

int	ImageDiff::getMaxY() const
{
	return (_maxY);
}

const std::vector<unsigned char>&	ImageDiff::getBeforeData() const
{
	return (_beforeData);
}

const std::vector<unsigned char>&	ImageDiff::getAfterData() const
{
	return (_afterData);
}

void	ImageDiff::setBoundingBox(int minX, int minY, int maxX, int maxY)
{
	this->_minX = minX;
	this->_minY = minY;
	this->_maxX = maxX;
	this->_maxY = maxY;
}

void	ImageDiff::setBeforeData(const std::vector<unsigned char>& data)
{
	this->_beforeData = data;
}

void	ImageDiff::setAfterData(const std::vector<unsigned char>& data)
{
	this->_afterData = data;
}

std::vector<unsigned char> ImageDiff::compressRLE(const unsigned char* data, int width, int /*height*/, int startX, int startY, int endX, int endY) {
    std::vector<unsigned char> compressed;
    
    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; ) {
            int pixelIndex = (y * width + x) * 4;
            unsigned char r = data[pixelIndex];
            unsigned char g = data[pixelIndex + 1];
            unsigned char b = data[pixelIndex + 2];
            unsigned char a = data[pixelIndex + 3];
            
            // 同じ色が続く長さを数える
            int runLength = 1;
            int nextX = x + 1;
            while (nextX <= endX && runLength < 255) {
                int nextIndex = (y * width + nextX) * 4;
                if (data[nextIndex] == r && data[nextIndex + 1] == g && 
                    data[nextIndex + 2] == b && data[nextIndex + 3] == a) {
                    runLength++;
                    nextX++;
                } else {
                    break;
                }
            }
            
            // RLEデータを追加
            compressed.push_back(runLength);
            compressed.push_back(r);
            compressed.push_back(g);
            compressed.push_back(b);
            compressed.push_back(a);
            
            x += runLength;
        }
    }
    
    return compressed;
}

void ImageDiff::decompressRLE(const std::vector<unsigned char>& compressed, unsigned char* data, int width, int /*height*/, int startX, int startY, int endX, int endY) {
    size_t compressedIndex = 0;
    int x = startX;
    int y = startY;
    
    while (compressedIndex < compressed.size() && y <= endY) {
        unsigned char runLength = compressed[compressedIndex++];
        unsigned char r = compressed[compressedIndex++];
        unsigned char g = compressed[compressedIndex++];
        unsigned char b = compressed[compressedIndex++];
        unsigned char a = compressed[compressedIndex++];
        
        for (int i = 0; i < runLength && y <= endY; i++) {
            if (x > endX) {
                x = startX;
                y++;
                if (y > endY) break;
            }
            
            int pixelIndex = (y * width + x) * 4;
            data[pixelIndex] = r;
            data[pixelIndex + 1] = g;
            data[pixelIndex + 2] = b;
            data[pixelIndex + 3] = a;
            x++;
        }
    }
}

void ImageDiff::serialize(std::ofstream& file) const {
    file.write(reinterpret_cast<const char*>(&_minX), sizeof(_minX));
    file.write(reinterpret_cast<const char*>(&_minY), sizeof(_minY));
    file.write(reinterpret_cast<const char*>(&_maxX), sizeof(_maxX));
    file.write(reinterpret_cast<const char*>(&_maxY), sizeof(_maxY));
    
    size_t beforeSize = _beforeData.size();
    size_t afterSize = _afterData.size();
    file.write(reinterpret_cast<const char*>(&beforeSize), sizeof(beforeSize));
    file.write(reinterpret_cast<const char*>(&afterSize), sizeof(afterSize));
    
    if (beforeSize > 0) {
        file.write(reinterpret_cast<const char*>(_beforeData.data()), beforeSize);
    }
    if (afterSize > 0) {
        file.write(reinterpret_cast<const char*>(_afterData.data()), afterSize);
    }
}

void ImageDiff::deserialize(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(&_minX), sizeof(_minX));
    file.read(reinterpret_cast<char*>(&_minY), sizeof(_minY));
    file.read(reinterpret_cast<char*>(&_maxX), sizeof(_maxX));
    file.read(reinterpret_cast<char*>(&_maxY), sizeof(_maxY));
    
    size_t beforeSize, afterSize;
    file.read(reinterpret_cast<char*>(&beforeSize), sizeof(beforeSize));
    file.read(reinterpret_cast<char*>(&afterSize), sizeof(afterSize));
    
    _beforeData.resize(beforeSize);
    _afterData.resize(afterSize);
    
    if (beforeSize > 0) {
        file.read(reinterpret_cast<char*>(_beforeData.data()), beforeSize);
    }
    if (afterSize > 0) {
        file.read(reinterpret_cast<char*>(_afterData.data()), afterSize);
    }
}

size_t ImageDiff::getSerializedSize() const {
    return sizeof(_minX) + sizeof(_minY) + sizeof(_maxX) + sizeof(_maxY) +
           sizeof(size_t) * 2 + _beforeData.size() + _afterData.size();
}