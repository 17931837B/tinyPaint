#include "ImageData.hpp"

ImageData::ImageData(int w, int h) : _width(w), _height(h)
{
	int	i;

	_imageData = new unsigned char[static_cast<size_t>(_width) * _height * 4]; //overflow対策
	i = 0;
	while (i < _width * _height)
	{
		_imageData[i * 4 + 0] = 255;
		_imageData[i * 4 + 1] = 0;
		_imageData[i * 4 + 2] = 0;
		_imageData[i * 4 + 3] = 255;
		i++;
	}
}

ImageData::~ImageData()
{
	delete[] _imageData;
}

ImageData::ImageData(const ImageData& other) : _width(other._width), _height(other._height)
{
	_imageData = new unsigned char[static_cast<size_t>(_width) * _height * 4];
	std::memcpy(_imageData, other._imageData, static_cast<size_t>(_width) * _height * 4);
}

ImageData& ImageData::operator=(const ImageData& src)
{
	if (this == &src)
		return (*this);
	delete[] _imageData;
	_width = src._width;
	_height = src._height;
	_imageData = new unsigned char[static_cast<size_t>(_width) * _height * 4];
	std::memcpy(_imageData, src._imageData, static_cast<size_t>(_width) * _height * 4);
	return (*this);
}

int	ImageData::getWidth() const
{
	return (_width);
}

int ImageData::getHeight() const
{
	return (_height);
}

unsigned char* ImageData::getImageData() const
{
	return (_imageData);
}
