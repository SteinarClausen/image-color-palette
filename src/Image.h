#pragma once
#include <cstdint>
#include <cstddef>


enum ImageType {
    PNG, JPG
};

class Color_quantization;

class Image;

class Pixel {
public:
    uint8_t* data;
    int channels;

    explicit Pixel(int channels);
    explicit Pixel(const Image& image);
    Pixel(const Pixel& pixel);
    Pixel& operator=(const Pixel& pixel);
    ~Pixel();

    void setPixelData(const uint8_t* data) const;
};

class Image {
private:
    uint8_t* data;
    size_t size;

public:
    int width{};
    int height{};
    int channels{};

    explicit Image(const char* filename);
    Image(int width, int height, int channels);
    Image(const Image& image);
    explicit Image(const Color_quantization& cq);
    Image& operator=(const Image& image);
    ~Image();

    bool read(const char* filename);
    bool write(const char* filename) const;

    static ImageType getFileType(const char* filename) ;

    uint8_t* getData() const { return data; }
    size_t getSize() const {return size; }

    void setData(int x, int y, const uint8_t* colorData);
    void setData(int x, int y, Pixel pixel);
    void setData(int index, uint8_t value);

    void getPixel(int x, int y, uint8_t* pixelData) const; //Passes reference to location for storing data.
    void getPixel(int x, int y, Pixel* pixel) const;
};


//lines changed Image.h: 1279, 4627