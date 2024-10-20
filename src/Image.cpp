#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"
#include "stb_image_write.h"
#include "Image.h"
#include "Image_color_quantization.h"
#include <iostream>
#include <cstring>

Image::Image(const char* filename) {
    if (!read(filename)) {
        std::cerr << "Couldn't open file: " << filename << std::endl;
    }
    size = width * height * channels;
}

Image::Image(int const width, int const height, int const channels) : width(width), height(height), channels(channels) {
    size = width * height * channels;
    data = new uint8_t[size];
}

Image::Image(const Image& image) : Image(image.width, image.height, image.channels) {
    if (image.data) {
        std::memcpy(data, image.data, size);
    }
}

Image::Image(const Color_quantization& cq) : Image(cq.width, cq.height, cq.channels) {
    if (cq.data) {
        std::memcpy(data, cq.data, size);
    }
}

Image::~Image() {
    if (data) {
        stbi_image_free(data);
    }
}

Image& Image::operator=(const Image& image) {
    if (this == &image) {
        return *this; // self-assignment check
    }
    delete[] data; // free the existing data

    width = image.width;
    height = image.height;
    channels = image.channels;
    size = image.size;
    data = new uint8_t[size];
    if (this->getData()) {
        std::memcpy(data, image.data, size);
    }

    return *this;
}

bool Image::read(const char* filename) {
    data = stbi_load(filename, &width, &height, &channels, 0);
    if (data == NULL) {
        std::cerr << "Error loading image: " << filename << " - " << stbi_failure_reason() << std::endl;
        return false;
    }
    size = width * height * channels;
    return true;
}

bool Image::write(const char* filename) const{
    ImageType const type = getFileType(filename);
    int success = 0;
    switch(type) {
        case PNG:
            success = stbi_write_png(filename, width, height, channels, data, width * channels);
            break;
        case JPG:
            success = stbi_write_jpg(filename, width, height, channels, data, 100);
            break;
    }
    return success != 0;
}

ImageType Image::getFileType(const char* filename) {
    const char* extension = strrchr(filename, '.');
    if (extension != nullptr) {
        if (strcmp(extension, ".png") == 0) {
            return PNG;
        } else if (strcmp(extension, ".jpg") == 0) {
            return JPG;
        }
    }
    return PNG; // Default to PNG if no extension matches
}

void Image::setData(int x, int y, const uint8_t* colorData) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        int const index = (y * width + x) * channels;
        for (int i = 0; i < channels; ++i) {
            data[index + i] = colorData[i];
        }
    }
}

void Image::setData(int const index, uint8_t const value) {
    if (index >= 0 && index < size) {
        data[index] = value;
    }
}

void Image::getPixel(int x, int y, uint8_t* pixelData) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        int const index = (y*width+x)*channels;
        for(int i = 0; i < channels; ++i) {
            pixelData[i] = data[index+i];
        }
    }
}

void Image::getPixel(int x, int y, Pixel* pixel) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        int const index = (y*width+x)*channels;
        for(int i = 0; i < channels; ++i) {
            pixel->data[i] = data[index+i];
        }
    }
}

Pixel::Pixel(int channels) : channels(channels) {
    data = new uint8_t[channels];
}

Pixel::Pixel(const Pixel& pixel) : channels(pixel.channels) {
    data = new uint8_t[channels];
    std::memcpy(data, pixel.data, channels);
}

Pixel::Pixel(const Image& image) : channels(image.channels) {
    data = new uint8_t[image.channels];
}

Pixel& Pixel::operator=(const Pixel& pixel) {
    if (this == &pixel) {
        return *this;
    }
        delete[] data;
        channels = pixel.channels;
        data = new uint8_t[channels];
        std::memcpy(data, pixel.data, channels);
    return *this;
}

Pixel::~Pixel() {
    delete[] data;
}

void Pixel::setPixelData(const uint8_t* data) const {
    std::memcpy(this->data, data, this->channels); //assumes that data is equal to channels in pixel
}
