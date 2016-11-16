#include <iostream>
#include "ocr.h"
using namespace cv;

int main(int argc, char **argv) {
    if (argc < 2){
        std::cout << "usage:\nocr_test_driver [filename]\n";
        return 2;
    }
    char *fn = argv[1];     //The image file we are trying to read 
    Mat img;
    img = imread(fn);

    std::vector<Rect> boxes = segment(img);
    for (auto i : boxes) std::cout <<i<<" "<<'\n';
    ocr_rois(boxes, img, fn);
}
