#include "opencv2/opencv.hpp"
#include <iostream>
#include <tesseract/baseapi.h>

using namespace cv;
using namespace std;


vector<Rect> segment(const Mat& image)
{
    Mat img = image;

    // Convert to grayscale.
    cvtColor(img, img, CV_BGR2GRAY);

    // Morphological gradient
    Mat morphKernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    morphologyEx(img, img, MORPH_GRADIENT, morphKernel);

    // Binarize
    threshold(img, img, 0.0, 255.0, THRESH_BINARY | THRESH_OTSU);

    // Connect horizontally oriented regions
    morphKernel = getStructuringElement(MORPH_RECT, Size(9, 1));
    morphologyEx(img, img, MORPH_CLOSE, morphKernel);

    // Find contours
    Mat mask = Mat::zeros(img.size(), CV_8UC1);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(img, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    // Filter contours
    vector<Rect> boxes;
    for(int idx = 0; idx >= 0; idx = hierarchy[idx][0])
    {
        Rect rect = boundingRect(contours[idx]);
        Mat maskROI(mask, rect);
        maskROI = Scalar(0, 0, 0);

        // Fill the contour
        drawContours(mask, contours, idx, Scalar(255, 255, 255), CV_FILLED);

        // Ratio of non-zero pixels in the filled region
        double r = (double)countNonZero(maskROI)/(rect.width*rect.height);

        // Assume at least 45% of the area is filled if it contains text.
        // And constrain the region size. It might be more robust to use something
        // like the number of significant pixels in a horizontal projection
        // as a third condition.
        if (r > .45 && (rect.height > 8 && rect.width > 8))
            boxes.push_back(rect);
    }
    return boxes;
}


void ocr_boxes(vector<Rect> boxes, const Mat& img)
{
    double avg_confidence = 0;
    size_t counter = 0;
    for (auto i : boxes) {
        cout << i.x << " " << i.y << " " << i.width << " " << i.height <<  endl;
        Mat roi = img(Rect(i.x, i.y, i.width, i.height));
        resize(roi, roi, roi.size()*4);
        tesseract::TessBaseAPI tess;
        tess.Init(NULL, "eng", tesseract::OEM_TESSERACT_CUBE_COMBINED);

        tess.SetImage((uchar*)roi.data,
                      roi.size().width,
                      roi.size().height,
                      roi.channels(), roi.step1());
        tess.Recognize(0);
        const char* out = tess.GetUTF8Text();

        // Don't count the whole image in the average.
        if (counter < boxes.size()) {
            avg_confidence += tess.MeanTextConf();
            counter++;
        }
        printf("confidence: %d\n", tess.MeanTextConf());
        cout << out << endl;
        cout << "\n";
    }
    printf("Average confidence: %5.2f\n", avg_confidence/counter);
}


int main(int argc, char* argv[])
{
    if (argc < 2) {
        perror("Usage: segment {imagename}\n");
        exit(-1);
    }
    else {
        Mat img = imread(argv[1]);
        vector<Rect> boxes;
        boxes = segment(img);
        ocr_boxes(boxes, img);
    }
    return 0;
}
