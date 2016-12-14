#ifndef UNLINE_H
#define UNLINE_H
#include "opencv2/opencv.hpp"
using namespace cv;

Mat unline(Mat src) {
    //imshow("src", src);
    Mat gray;
    if (src.channels() == 3) {
        cvtColor(src, gray, CV_BGR2GRAY);
    }
    else {
        gray = src;
    }
    //imshow("gray", gray);

    Mat bw;
    adaptiveThreshold(~gray, bw, 255, CV_ADAPTIVE_THRESH_MEAN_C,
            THRESH_BINARY, 15, 0);

    imshow("binary", bw);
    Mat horizontal = bw.clone();
    Mat vertical = bw.clone();

    int horizontalsize = horizontal.cols / 30;
    Mat horizontalStructure = getStructuringElement(MORPH_RECT, Size(horizontalsize, 1));

    erode(horizontal, horizontal, horizontalStructure, Point(-1,-1));
    dilate(horizontal, horizontal, horizontalStructure, Point(-1,-1));
    //imshow("horizontal", horizontal);

    int verticalsize = vertical.cols / 30;
    Mat verticalStructure = getStructuringElement(MORPH_RECT, Size(1, verticalsize));
    erode(vertical, vertical, verticalStructure, Point(-1,-1));
    dilate(vertical, vertical, verticalStructure, Point(-1,-1));
    //imshow("vertical", vertical);

    //bitwise_not(vertical, vertical);
    //imshow("vetical_bit", vertical);

    Mat edges;
    adaptiveThreshold(vertical, edges, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, -2);
    //imshow("edges", edges);

    Mat kernel = Mat::ones(2, 2, CV_8UC1);
    dilate(edges, edges, kernel);
    //imshow("dilate", edges);

    Mat smooth;
    vertical.copyTo(smooth);
    blur(smooth, smooth, Size(3, 3));
    //imshow("smooth", vertical);

    Mat xr;
    multiply(gray, vertical, xr);
    //multiply(xr, horizontal, xr);
    //imshow("xor", xr);
    //waitKey(-1);
    return xr;
}

#endif //END UNLINE_H
