#ifndef IMAGEDATA_HPP
#define IMAGEDATA_HPP

#include <iostream>

class ImageData {
private:
    int             width;
    int             height;
    unsigned char* imageData;

public:
    // コンストラクタ
    ImageData(int w, int h) : width(w), height(h) {
        imageData = new unsigned char[width * height * 4];
        // 必要に応じて初期化処理を追加
    }

    // デストラクタ
    ~ImageData() {
        delete[] imageData;
    }

    // コピーコンストラクタ
    ImageData(const ImageData& other) {
        width = other.width;
        height = other.height;
        imageData = new unsigned char[width * height * 4];
        std::copy(other.imageData, other.imageData + width * height * 4, imageData);
    }

    // 代入演算子
    ImageData& operator=(const ImageData& other) {
        if (this != &other) {
            delete[] imageData;
            width = other.width;
            height = other.height;
            imageData = new unsigned char[width * height * 4];
            std::copy(other.imageData, other.imageData + width * height * 4, imageData);
        }
        return *this;
    }

    // アクセサメソッド
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    unsigned char* getImageData() const { return imageData; }
};

#endif