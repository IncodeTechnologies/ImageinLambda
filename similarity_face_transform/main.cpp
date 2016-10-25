#include <iostream>
#include <opencv2/opencv.hpp>
#include "SimilarityTransform.h"

using namespace cv;

cv::Size imgSize = {96, 112};
std::vector<cv::Point2f> coord5points = {
    {30.2946f, 51.6963f},
    {65.5318f, 51.5014f},
    {48.0252f, 71.7366f},
    {33.5493f, 92.3655f},
    {62.7299f, 92.2041f}
};

cv::Mat transform_face(cv::Mat img, const std::vector<cv::Point2f>& img5Points) {
    cv::Mat aT = calcSimilarityTransform(img5Points, coord5points);
    for (int i = 0; i < aT.rows; ++i) {
        for (int j = 0; j < aT.cols; ++j) {
            std::cout << aT.at<float>(i,j) << " ";
        }
        std::cout << std::endl;
    }
    cv::Mat dstImg;
    cv::warpAffine(img, dstImg, aT, imgSize);
    return dstImg;
}

int main(int argc, const char * argv[]) {
    cv::Mat img = cv::imread("source_image.jpg", CV_LOAD_IMAGE_COLOR);
    std::vector<cv::Point2f> img5Points = {
        {105.415f, 111.097f},
        {146.245f, 121.271f},
        {121.668f, 139.903f},
        {105.019f, 151.002f},
        {129.596f, 157.873f}
    };
    cv::Mat dstImg = transform_face(img, img5Points);
    cv::imwrite("target_image.jpg", dstImg);
    return 0;
}
