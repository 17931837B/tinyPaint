#ifndef IMAGEDIFF_HPP
#define IMAGEDIFF_HPP

#include <vector>
#include <fstream>

class ImageDiff
{
	private:
		int	_minX;
		int	_minY;
		int	_maxX;
		int	_maxY;
		std::vector<unsigned char>	_beforeData;
		std::vector<unsigned char>	_afterData;
	public:
		ImageDiff();
		static std::vector<unsigned char>	compressRLE(const unsigned char* data, int width, int height, int startX, int startY, int endX, int endY);
		static void							decompressRLE(const std::vector<unsigned char>& compressed, unsigned char* data, int width, int height, int startX, int startY, int endX, int endY);
		void								serialize(std::ofstream& file) const;
		void								deserialize(std::ifstream& file);
		size_t								getSerializedSize() const;
		int									getMinX() const;
		int									getMinY() const;
		int									getMaxX() const;
		int									getMaxY() const;
		const std::vector<unsigned char>&	getBeforeData() const;
		const std::vector<unsigned char>&	getAfterData() const;
		void								setBoundingBox(int minX, int minY, int maxX, int maxY);
		void								setBeforeData(const std::vector<unsigned char>& data);
		void								setAfterData(const std::vector<unsigned char>& data);
};

#endif
