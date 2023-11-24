#include "net.h"
#include <opencv2/core/core.hpp>


struct FaceObject {
    cv::Rect_<float> rect;
    cv::Point2f landmark[5];
    float prob;
};

extern std::vector<FaceObject> detect_scrfd(const ncnn::Net & scrfd, ncnn::Mat in1);
