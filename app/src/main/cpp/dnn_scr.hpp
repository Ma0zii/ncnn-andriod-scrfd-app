#include <opencv2/core/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

extern std::tuple<cv::Mat, int, int> resizeNorm(cv::Mat & oriImg);

extern std::tuple<std::vector<int>, std::vector<int>, std::vector<float>> detect_dnn(cv::dnn::Net net1, cv::Mat in1, int ih, int iw, int padh, int padw);