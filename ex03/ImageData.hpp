#ifndef IMAGEDATA_HPP
#define IMAGEDATA_HPP

#include <iostream>
#include <algorithm>
#include <cstring>

class	ImageData
{
	private:
		int				_width;
		int				_height;
		unsigned char*	_imageData;

	public:
		ImageData(int w, int h);
		~ImageData();
		ImageData(const ImageData& other);
		ImageData& operator=(const ImageData& other);
		int getWidth() const;
		int getHeight() const;
		unsigned char* getImageData() const;
};

#endif