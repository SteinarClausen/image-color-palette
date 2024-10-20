#include "Image.h"
#include "Image_color_quantization.h"
#include <iostream>

#include <filesystem>

int main() {

    const char* filename = "C:/Users/stein/cplusplus/image_quantization/src/Nidarosdomen.jpg";

    if (std::filesystem::exists(filename)) {
        std::cout << "File exists" << filename << std::endl;
    } else {
        std::cout << "no" << std::endl;
    }

    Image test(filename);
    Color_quantization cq(test);
    cq.median_cut_algorithm(4);
    cq.palette_color_quantization();
    for(int i = 0; i < cq.color_amount_parts.size(); i++)
        std::cout << cq.color_amount_parts[i] << ' ';
    Image img(cq);

    img.write("C:/Users/stein/cplusplus/image_quantization/src/Nidarosdomen15.png");

    Image copy = test;
    uint8_t tempcolor[] = {123,50,200};
    for (int i = 0; i < copy.width; ++i) {
            copy.setData(i,0,tempcolor);
    }
    copy.write("C:/Users/stein/cplusplus/image_quantization/src/copy2.jpg");
    Image blank(100, 100, 3);
    blank.write("C:/Users/stein/cplusplus/image_quantization/src/blank2.jpg"); //TODO: figure out why blank is altered - stb_image not working?
    return 0;
}
