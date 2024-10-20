#include "stb_image.h"
#include "Image_color_quantization.h"

#include <algorithm>
#include <cmath>

#include "Image.h"
#include <cstring>
#include <utility>
#include <vector>
#include <iostream>

Color_quantization::Color_quantization(const Image& img) : width(img.width), height(img.height), channels(img.channels) {
    size = img.getSize();
    if (img.getData()) {
        data = new uint8_t[size];
        std::memcpy(data, img.getData(), size);
    } else {
        data = nullptr;
    }
    color_palette = new uint8_t[channels];
    simplified_data = new int[width*height];
}

Color_quantization::~Color_quantization() {
    if (data) {
        stbi_image_free(data);
    }
    delete[] color_palette;
    delete[] simplified_data;
}

void Color_quantization::swap(int x1, int x2) const {
    for (int i = 0; i < channels; ++i) {
        std::swap(data[x1+i], data[x2+i]);
    }
}


void Color_quantization::median_cut_algorithm(int colors) {
    set_color_amount(colors);
    buckets.clear();
    Bucket b(*this);
    buckets.push_back(b);
    for (int const parts : color_amount_parts) {
        for (int i = buckets.size() -1; i >= 0; --i){
            buckets[i].sort_by_channel(buckets[i].get_sort_color());
            median_cut(buckets[i], parts, i);
        }
    }
    for (Bucket& bucket: buckets)
    {
        bucket.sort_by_channel(bucket.get_sort_color());
        bucket.find_mean_color();
        std::cout << static_cast<int>(bucket.mean_color[0]) << " " << static_cast<int>(bucket.mean_color[1]) << " " << static_cast<int>(bucket.mean_color[2]) << " " << std::endl;
        for (int i = 0; i < bucket.size/bucket.channels; ++i) {
            for (int j = 0; j < channels; ++j) {
                bucket.data[i*channels+j] = bucket.mean_color[j];
            }
        }
    }
    find_color_palette();
}

void Color_quantization::median_cut(Bucket const& bucket_c, int const parts, int const iterator_to_replace) {
    Bucket bucket(bucket_c);     //copy bucket_c because it will be replaced

    size_t const num_pixels = bucket.size / channels;
    std::vector<size_t> sizes(parts, num_pixels / parts);
    size_t rest = num_pixels % parts;
    for (int i = 0; i < rest; ++i) {
        sizes.at(i) += 1;
    }

    for (size_t& size : sizes) {    // Convert sizes from number of pixels to bytes
        size *= channels;
    }

    size_t current_iterator = 0;
    for (int i = 0; i < parts; ++i) {
        size_t current_part_size = sizes[i];

        Bucket temp_bucket(sizes[i], channels);  // Create a bucket with the correct number of pixels

        if (current_iterator + current_part_size > bucket.size) {
        }

        std::memcpy(temp_bucket.data, bucket.data + current_iterator, current_part_size);
        current_iterator += current_part_size;

        if (i == 0) {
            buckets[iterator_to_replace] = std::move(temp_bucket);
        } else {
            buckets.insert(buckets.begin() + iterator_to_replace + i, std::move(temp_bucket));
        }
    }
}

void Color_quantization::find_color_palette() {
    delete[] color_palette;
    color_palette = new uint8_t[color_amount*channels];
    size_t iterator = 0;
    for (const Bucket& bucket : buckets) {
        std::memcpy(color_palette + iterator, bucket.mean_color, bucket.channels);
        iterator += bucket.channels;
    }
}

void Color_quantization::return_buckets_to_data() {
    size_t iterator = 0;
    for (const Bucket& bucket : buckets) {
        std::memcpy(data + iterator, bucket.data, bucket.size);
        iterator += bucket.size;
    }
}

void Color_quantization::find_primes(int number, bool const flipped) {
    for (int i = 2; i <= number; ++i) {
        while (number % i == 0) {
            color_amount_parts.push_back(i);
            number /= i;
        }
    }
    if (flipped) {
        std::reverse(color_amount_parts.begin(), color_amount_parts.end());
    }
}

void Color_quantization::set_color_amount(int const colors) {
    color_amount = colors;
    find_primes(colors, true);
}

int Color_quantization::find_color_distance(uint8_t const pixel_r, uint8_t const pixel_g, uint8_t const pixel_b, uint8_t const palette_r,uint8_t const palette_g,uint8_t const palette_b) {
    int16_t const red_dif = static_cast<int16_t>(pixel_r)-static_cast<int16_t>(palette_r);
    int16_t const green_dif = static_cast<int16_t>(pixel_g)-static_cast<int16_t>(palette_g);
    int16_t const blue_dif = static_cast<int16_t>(pixel_b)-static_cast<int16_t>(palette_b);
    return static_cast<int32_t>(red_dif)*red_dif+static_cast<int32_t>(green_dif)*green_dif+static_cast<int32_t>(blue_dif)*blue_dif; //original formula finds the square root of d^2, but in this program the exact distance is not necessary, so computing power is saved
}

void Color_quantization::palette_color_quantization(bool quantize_image) {
    bool rgb = (channels != 1); // false if black/white, true if not
    for (int i = 0; i < size; i += channels) {
        int32_t smallest_color_distance = 65536;
        int palette_color_number = 0;
        int32_t color_distance;
        if (rgb) {
            for (int j = 0; j < color_amount; ++j) {
                color_distance = find_color_distance(data[i], data[i+1], data[i+2], color_palette[j*channels],color_palette[j*channels+1],color_palette[j*channels+2]);
                if (color_distance < smallest_color_distance) {
                    smallest_color_distance = color_distance;
                    palette_color_number = j;
                }
            }
        } else {
            for (int j = 0; j < color_amount; ++j) {
                color_distance = std::abs(static_cast<int16_t>(data[i])-static_cast<int16_t>(color_palette[j]));
                if (color_distance < smallest_color_distance) {
                    smallest_color_distance = color_distance;
                    palette_color_number = j;
                }
            }
        }
        if (quantize_image) {
            for (int k = 0; k < channels; ++k) {
                data[i+k] = color_palette[palette_color_number*channels+k];
            }
        }
        simplified_data[i / channels] = palette_color_number; //contains one number for the color, no channels
    }
}

void Color_quantization::simplified_data_to_data() {
    for (int i = 0; i < width*height; ++i) {
        std::memcpy(data + i*channels, color_palette + simplified_data[i]*channels, channels);
    }
}

Bucket::Bucket(size_t const size, int const channels) : data(new uint8_t[size]), mean_color(new uint8_t[channels]), size(size), channels(channels) {
    std::memset(mean_color,0,channels);
}

Bucket::Bucket(const Color_quantization& cq) : data(new uint8_t[cq.size]), mean_color(new uint8_t[cq.channels]), size(cq.size), channels(cq.channels) {
    if (cq.data) {
        std::memcpy(data, cq.data, size);
    } else {
        data = new uint8_t[size];
    }
    std::memset(mean_color,0,channels);
}

Bucket::~Bucket() {
    delete[] data;
    delete[] mean_color;
}

Bucket::Bucket(const Bucket& other) : data(new uint8_t[other.size]), mean_color(new uint8_t[other.channels]), size(other.size), channels(other.channels) {
    std::memcpy(data, other.data, other.size);
    std::memcpy(mean_color, other.mean_color, other.channels);
}

Bucket::Bucket(Bucket&& other) noexcept : data(other.data), mean_color(other.mean_color), size(other.size), channels(other.channels) {
    other.data = nullptr;
    other.mean_color = nullptr;
    other.size = 0;
    other.channels = 0;
}

Bucket& Bucket::operator=(const Bucket& other) {
    if (this != &other) {
        delete[] data;
        delete[] mean_color;
        size = other.size;
        channels = other.channels;
        data = new uint8_t[size];
        mean_color = new uint8_t[channels];
        std::memcpy(data, other.data, size);
        std::memcpy(mean_color, other.mean_color, channels);
    }
    return *this;
}

Bucket& Bucket::operator=(Bucket&& other) noexcept {
    if (this != &other) {
        delete[] data;
        delete[] mean_color;
        data = other.data;
        mean_color = other.mean_color;
        size = other.size;
        channels = other.channels;
        other.data = nullptr;
        other.mean_color = nullptr;
        other.size = 0;
        other.channels = 0;
    }
    return *this;
}

int Bucket::get_sort_color() const {
    int color_channels;
    std::vector<uint8_t> color_diff;

    if (channels > 3) {
        color_channels = 3;
    } else {
        color_channels = channels;
    }

    color_diff.resize(color_channels, 0);

    for (int i = 0; i < color_channels; ++i) {
        uint8_t smallest_color_value = data[i];
        uint8_t largest_color_value = data[i];
        int j = i + channels;

        while (j < size) {
            if (data[j] < smallest_color_value) {
                smallest_color_value = data[j];
            } else if (data[j] > largest_color_value) {
                largest_color_value = data[j];
            }
            j += channels;
        }
        if (largest_color_value-smallest_color_value == 255) {
            return i;
        }
        color_diff[i] = largest_color_value - smallest_color_value;
    }

    int sort_channel = 0;
    for (int i = 1; i < color_channels; ++i) {
        if (color_diff[i] > color_diff[sort_channel]) {
            sort_channel = i;
        }
    }

    return sort_channel;
}

void Bucket::sort_by_channel(int const channel) { //counting sort
    if (channel >= channels) {
        return;
    }
    auto* sorted_data = new uint8_t[size];
    int count[256] = {0}; // Array to store the count of each color value (0-255)

    for (int i = channel; i < size; i += channels) { //count R,G,B amounts
        count[data[i]]++;
    }

    for (int i = 1; i < 256; ++i) {  //add up
        count[i] += count[i - 1];
    }

    for (int i = size - channels; i >= 0; i -= channels) { //sort pixel by channel
        int const color_value = data[i + channel];
        int const sorted_index = (count[color_value] - 1) * channels;

        for (int j = 0; j < channels; ++j) {         // Place the entire pixel in the correct position TODO: use function instead
            sorted_data[sorted_index + j] = data[i + j];
        }

        count[color_value]--;
    }

    std::memcpy(data, sorted_data, size); //copy back to original
    delete[] sorted_data;
}

void Bucket::find_mean_color() const {
    auto* temp_mean_color = new double[channels];
    std::memset(temp_mean_color, 0, sizeof(double) * channels);

    for (size_t i = 0; i < size; i += channels) { // Convert from a floating point to RGB
        for (int j = 0; j < channels; ++j) {
            double value = data[i + j] / 255.0; // 1 for 255, 0 for 0
            if (value <= 0.04045) { //https://en.wikipedia.org/wiki/SRGB#Transformation
                temp_mean_color[j] += value / 12.92;
            } else {
                temp_mean_color[j] += pow((value + 0.055) / 1.055, 2.4);
            }
        }
    }

    for (int j = 0; j < channels; ++j) {
        temp_mean_color[j] /= (size / channels);
    }

    for (int j = 0; j < channels; ++j) { // Convert from a floating point to RGB
        if (temp_mean_color[j] <= 0.0031308) {
            mean_color[j] = static_cast<uint8_t>(round(12.92 * temp_mean_color[j] * 255.0));
        } else {
            mean_color[j] = static_cast<uint8_t>(round((1.055 * pow(temp_mean_color[j], 1.0 / 2.4) - 0.055) * 255.0));
        }
    }
    delete[] temp_mean_color;
}

