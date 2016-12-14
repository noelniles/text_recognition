#include "ocr.h"
#include "unline.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <openssl/sha.h>
#include <random>
using namespace cv;


void create_log(std::string &lf, const std::string &imgfilename) {
    time_t rawtime;
    struct tm *utc;
    time(&rawtime);
    utc = gmtime(&rawtime);

    char date[80];
    strftime(date, 80, "%Y/%m/%d", utc);

    char time[80];
    strftime(time, 80, "%H:%M:%S", utc);


    Mat img;
    img = imread(imgfilename);
    std::string fn = imhash(img);
    std::string hash = imhash(img);
    std::string logpath = "./data/logs/"+std::string(fn);
    std::ofstream logfile;
    logfile.open(logpath);

    if (!logfile.is_open()) {
        std::cout << "problem opening log file\n" << std::endl;
    }
    else {
        logfile << "image_filename: " << imgfilename << '\n';
        logfile << "run_date: " << date << '\n';
        logfile << "run_time: " << time << "\n";
        logfile << "hash: " << hash << "\n";
        logfile << "region confidence result\n";
    }
    lf = "./data/logs/"+std::string(fn);
    logfile.close();
}

/* Hash an image */
std::string imhash(const Mat& img) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(img.data, sizeof(img) - 1, hash);
    std::cout << "Hash: "<< std::hex << &hash << std::endl;
    return std::string((char*)&hash);
}

/* Record the log */
void record(std::string &lf, std::string &record) {
    std::ofstream logfile;
    logfile.open(lf);
    logfile << record;
}

/*****************************************************************************
 * Given an a Mat containing image data.
 * Find regions containing text and return a vector containing bounding boxes.
 ****************************************************************************/
std::vector<Rect> segment(const Mat& image)
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
    morphKernel = getStructuringElement(MORPH_RECT, Size(17, 3));
    morphologyEx(img, img, MORPH_CLOSE, morphKernel);
    morphologyEx(img, img, MORPH_OPEN, morphKernel);

    // Find contours
    Mat mask = Mat::zeros(img.size(), CV_8UC1);
    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;
    findContours(img, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(3, 0));

    // Filter contours
    std::vector<Rect> boxes;
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


/*****************************************************************************
 * Given a vector of bounding boxes, a Mat containing image data, and 
 * a filename.
 * Process each ROI using Tesseract OCR.
 ****************************************************************************/
void ocr_rois(std::vector<Rect> boxes, const Mat& img, std::string imgfilename)
{
    std::string logfile;
    create_log(logfile, imgfilename);

    double avg_confidence = 0;
    size_t counter = 0;

    std::ofstream log;
    log.open(logfile, std::ios::app);

    for (auto i : boxes) {
        Mat roi = img(Rect(i.x, i.y, i.width, i.height));
        resize(roi, roi, roi.size()*6);

        //Uncomment these to look at the cropped regions. */
        //imshow("image", roi);
        //waitKey(0);
        tesseract::TessBaseAPI tess;
        tess.Init(NULL, "eng", tesseract::OEM_TESSERACT_CUBE_COMBINED);

        tess.SetImage((uchar*)roi.data,
                      roi.size().width,
                      roi.size().height,
                      roi.channels(), roi.step1());
        tess.Recognize(0);

        const char* out = tess.GetUTF8Text();
        log << i.x << " " << i.y << " " << i.width << " "
            << i.height << " " << " " << tess.MeanTextConf() << " " << out << '\n';
        // Don't count the whole image in the average.
        if (counter < boxes.size()) {
            avg_confidence += tess.MeanTextConf();
            counter++;
        }
        std::cout << out << '\n' << std::endl;
    }
    log.close();
}
