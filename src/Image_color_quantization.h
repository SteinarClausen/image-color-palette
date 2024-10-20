#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>

class Image;

class Color_quantization;

class Bucket {
public:
    uint8_t* data;
    uint8_t* mean_color{};
    size_t size;
    int channels;

    Bucket(size_t size, int channels);
    explicit Bucket(const Color_quantization& cq);

    ~Bucket();

    Bucket(const Bucket& other);
    Bucket(Bucket&& other) noexcept;
    Bucket& operator=(const Bucket& other);
    Bucket& operator=(Bucket&& other) noexcept;

    int get_sort_color() const;
    void sort_by_channel(int channel);

    void find_mean_color() const;
};

class Color_quantization {
public:
    uint8_t* data{};
    int* simplified_data{};
    std::vector<Bucket> buckets{};
    size_t size{};
    uint8_t* color_palette{};
    int color_amount{};
    std::vector<int> color_amount_parts{};

    int width{};
    int height{};
    int channels{};

    explicit Color_quantization(const Image& img);
    ~Color_quantization();

    void swap(int x1, int x2) const; //swaps entire pixel

    // int get_sort_color() const; //123
    // void sort_by_channel(int const channel); //123

    void median_cut_algorithm(int colors);

    void median_cut(const Bucket& bucket, int const parts, int const iterator_to_replace = 0);

    // void find_mean_color(Bucket& bucket); //123

    void find_color_palette();

    void set_color_amount(int colors);
    void find_primes(int number, bool flipped = false);
    static int find_color_distance(uint8_t pixel_r, uint8_t pixel_g, uint8_t pixel_b, uint8_t palette_,uint8_t palette_g,uint8_t palette_b);

    void palette_color_quantization(bool quantize_image = true);

    void return_buckets_to_data();
    void simplified_data_to_data();
};
