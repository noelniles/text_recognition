#ifndef OCR_H
#define OCR_H
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
#include "tesseract/baseapi.h"
using namespace cv;


/**
 * Create the log file to keep track of OCR results. The file is created using
 * the current time for the filename
 */
void create_log(std::string &lf);


std::string imhash(const Mat &img);


/**
 * Record the result from ocring the current box. */
void record(std::string &lf, std::string &record);


/** 
 * Examine a form and find regions that are likely to be text. This method uses
 * the aspect ration of the found contours to determine whether a region contains
 * text or not. Regions that are longer than they are tall are good candidates
 * for being text. These regions are cropped out and returned in a vector that
 * can later be fed to ocr_rois() function.
 */
std::vector<cv::Rect> segment(const cv::Mat& img);


/**
 * Given a vector of regions of interest (ROI) containing text, process each
 * region using tesseract.
 */
void ocr_rois(std::vector<cv::Rect> rois, const cv::Mat& img, std::string imgfilename);

#endif //OCR_H
