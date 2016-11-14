//
//  main.cpp
//  parse_umdfaces
//
//  Created by Alexey Golunov on 14/11/2016.
//  Copyright © 2016 Alexey Golunov. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <map>

#include <opencv2/opencv.hpp>

#include "utils.hpp"

const int ec_mc_y = 48;
const int ec_y = 48;
const cv::Size outSize(128, 128);

inline cv::Mat alignFace(cv::Mat img, const std::vector<cv::Point2f>& pts) {

    cv::Point2f rEyeC(pts[7].x, pts[7].y);
    cv::Point2f lEyeC(pts[10].x, pts[10].y);
    cv::Point2f rMouth(pts[17].x, pts[17].y);
    cv::Point2f lMouth(pts[19].x, pts[19].y);
    cv::Point2f noseTip(pts[14].x, pts[14].y);

    cv::Mat imgOrig = img.clone();

    float ang_tan = (lEyeC.y - rEyeC.y) / (lEyeC.x - rEyeC.x);
    float ang = atan(ang_tan) / CV_PI * 180.0f;
    cv::Mat r = cv::getRotationMatrix2D(cv::Point2f(imgOrig.cols / 2, imgOrig.rows/2), ang, 1.0f);
    cv::Mat imgOrig_rot;
    cv::warpAffine(imgOrig, imgOrig_rot, r, cv::Size(imgOrig.cols, imgOrig.rows));

    // Rotate eyes center point
    cv::Point2f eyesC((rEyeC.x + lEyeC.x) / 2.0f, (rEyeC.y + lEyeC.y) / 2.0f);
    cv::Point2f eyesC_rot;
    eyesC_rot.x = (r.at<double>(0, 0)*eyesC.x + r.at<double>(0, 1)*eyesC.y + r.at<double>(0, 2));
    eyesC_rot.y = (r.at<double>(1, 0)*eyesC.x + r.at<double>(1, 1)*eyesC.y + r.at<double>(1, 2));
    // Rotate mouth center point
    cv::Point2f mouthC((rMouth.x + lMouth.x) / 2.0f, (rMouth.y + lMouth.y) / 2.0f);
    cv::Point2f mouthC_rot;
    mouthC_rot.x = (r.at<double>(0, 0)*mouthC.x + r.at<double>(0, 1)*mouthC.y + r.at<double>(0, 2));
    mouthC_rot.y = (r.at<double>(1, 0)*mouthC.x + r.at<double>(1, 1)*mouthC.y + r.at<double>(1, 2));

    // Resize to given ec_mc_y
    float resize_scale = static_cast<float>(ec_mc_y) / (mouthC.y - eyesC.y);
    cv::Mat imgOrig_resized;
    cv::resize(imgOrig_rot, imgOrig_resized, cv::Size(imgOrig_rot.cols * resize_scale, imgOrig_rot.rows * resize_scale));
    cv::Point2f eyesC2;
    eyesC2.x = eyesC.x * resize_scale;//(eyesC.x - imgOrig_rot.cols / 2) * resize_scale + imgOrig_resized.cols / 2;
    eyesC2.y = eyesC.y * resize_scale;//(eyesC.y - imgOrig_rot.rows / 2) * resize_scale + imgOrig_resized.rows / 2;

    // Crop
    int crop_y = std::max(0, (int)eyesC2.y - ec_y);
    int crop_y_end = std::min(imgOrig_resized.rows - 1, crop_y + outSize.height);
    int crop_x = std::max(0, (int)eyesC2.x - outSize.width / 2);
    int crop_x_end = std::min(imgOrig_resized.cols - 1, crop_x + outSize.width);

    cv::Mat fromROI = cv::Mat(imgOrig_resized, cv::Range(crop_y, crop_y_end), cv::Range(crop_x, crop_x_end));
    cv::Mat cropped_img(outSize, imgOrig_resized.type(),cv::Scalar(0));
    cv::Mat toROI = cv::Mat(cropped_img, cv::Range(0, crop_y_end - crop_y), cv::Range(0, crop_x_end - crop_x));
    fromROI.copyTo(toROI);
    return cropped_img;
}

struct UMDImageInfo {
    int personID;
    std::string fileName;
    cv::Rect faceRect;
    std::vector<cv::Point2f> points;

    UMDImageInfo() = default;
    UMDImageInfo(const std::string& str, const std::string& imagesPath, int offset) {
        auto vecOfValues = split(str, ',');
        if (vecOfValues.size() < 76) {
            throw std::string("Error: invalid input string");
        }
        personID = std::stoi(vecOfValues[0]) - 1 + offset;
        fileName = imagesPath + vecOfValues[1];
        faceRect.x = std::stof(vecOfValues[4]);
        faceRect.y = std::stof(vecOfValues[5]);
        faceRect.width = std::stof(vecOfValues[6]);
        faceRect.height = std::stof(vecOfValues[7]);

        for (int i = 0; i < 21; ++i) {
            cv::Point2f p;
            p.x = std::stof(vecOfValues[11 + 3 * i + 0]);
            p.y = std::stof(vecOfValues[11 + 3 * i + 1]);
            points.push_back(p);
        }
    }
};

void parseCSV(const std::string& csvFilePath,
              const std::string& imagesPath,
              std::map<int, std::vector<UMDImageInfo>>& result) {
    int offset = result.size();
    std::ifstream ifs(csvFilePath);
    while (ifs.good()) {
        try {
            std::string str = "";
            std::getline(ifs, str);
            UMDImageInfo iinf(str, imagesPath, offset);
            result[iinf.personID].push_back(iinf);
            std::cout << iinf.personID << " " << iinf.fileName << std::endl;
        } catch (...) {
            std::cout << "Unknown error" << std::endl;
        }
    }
    ifs.close();
}

int main(int argc, const char * argv[]) {
    std::string outImagePath = "/data/umdfaces_cropped/";

    std::vector<std::string> csvFilePaths = {"/data/umdfaces_batch1/umdfaces_batch1_ultraface.csv",
                                             "/data/umdfaces_batch2/umdfaces_batch2_ultraface.csv",
                                             "/data/umdfaces_batch3/umdfaces_batch3_ultraface.csv"
                                            };
    std::vector<std::string> imagesPaths = {"/data/umdfaces_batch1/",
                                            "/data/umdfaces_batch2/",
                                            "/data/umdfaces_batch3/"
                                           };
    std::map<int, std::vector<UMDImageInfo>> umdImages;
    for (size_t i = 0; i < csvFilePaths.size(); ++i) {
        parseCSV(csvFilePaths[i], imagesPaths[i], umdImages);
    }
    std::cout << umdImages.size() << std::endl;
    for (auto it = umdImages.begin(); it != umdImages.end(); ++it) {
        std::string personID = getStringID(it->first, 7);
        for (size_t i = 0; i < it->second.size(); ++i) {
            std::string imageID = getStringID(static_cast<int>(i), 5);
            UMDImageInfo& iinf = it->second[i];
            cv::Mat img = cv::imread(iinf.fileName);
            if (img.empty()) {
                std::cout << "Error: invalid image" << std::endl;
                continue;
            }
            cv::Mat alignedImg = alignFace(img, iinf.points);
            std::string outFileName = outImagePath + personID + std::string("_") + imageID + std::string(".jpg");
            cv::imwrite(outFileName, alignedImg);
        }
    }

    return 0;
}
