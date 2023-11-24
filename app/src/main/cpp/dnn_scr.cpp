//
// Created by 111 on 2023/11/23.
//
#include "dnn_scr.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

std::tuple<cv::Mat, int, int> resizeNorm(cv::Mat &oriImg)
{
    int width = oriImg.cols;
    int height = oriImg.rows;

    const int target_size = 640;

    // pad to multiple of 32
    int w = width;
    int h = height;
    int flag;
    float scale = 1.f;
    if (w > h)
    {
        scale = (float)target_size / w;
        w = target_size;
        h = h * scale;
        flag = 1;
    }
    else
    {
        scale = (float)target_size / h;
        h = target_size;
        w = w * scale;
        flag = 0;
    }

    cv::Mat resizedImg;
    cv::resize(oriImg, resizedImg, cv::Size(w, h));

    int h2 = 0;
    int w2 = 0;

    if(flag)
    {
        h2 = (target_size - h) / 2;
        copyMakeBorder(resizedImg, resizedImg, h2, target_size - h - h2, 0, 0, cv::BORDER_CONSTANT, 0);
    }
    else
    {
        w2 = (target_size - w) / 2;
        copyMakeBorder(resizedImg, resizedImg, 0, 0, w2, target_size - w - w2, cv::BORDER_CONSTANT, 0);
    }

    return std::make_tuple(resizedImg, h2, w2);
}

std::tuple<std::vector<int>, std::vector<int>, std::vector<float>> detect_dnn(cv::dnn::Net net1, cv::Mat in1, int ih, int iw, int padh, int padw)
{
    int t_size = 640;
    float confThreshold = 0.5f;
    float nmsThreshold = 0.5f;
    float stride[3] = { 8.0, 16.0, 32.0 };

    cv::Mat blob;
    cv::dnn::blobFromImage(in1, blob, 1 / 128.0, cv::Size(t_size, t_size), cv::Scalar(127.5, 127.5, 127.5), true, false);
    net1.setInput(blob);
    std::vector<cv::Mat> outs;
    net1.forward(outs, net1.getUnconnectedOutLayersNames());

    /////generate proposals
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    std::vector< std::vector<int>> landmarks;

    float ratioh = (float)ih / t_size;
    float ratiow = (float)iw / t_size;

    int n = 0, i = 0, j = 0, k = 0, l = 0;
    for (n = 0; n < 3; n++)   ///�߶�
    {
        int num_grid_x = (int)(t_size / stride[n]);
        int num_grid_y = (int)(t_size / stride[n]);
        float* pdata_score = (float*)outs[n].data;  ///score
        float* pdata_bbox = (float*)outs[n + 3 * 1].data;  ///bounding box
        float* pdata_kps = (float*)outs[n + 3 * 2].data;  ///face landmark

        for (i = 0; i < num_grid_y; i++)
        {
            for (j = 0; j < num_grid_x; j++)
            {
                for (k = 0; k < 2; k++)
                {
                    if (pdata_score[0] > confThreshold)
                    {
                        const int xmin = (int)(((j - pdata_bbox[0]) * stride[n] - padw) * ratiow);
                        const int ymin = (int)(((i - pdata_bbox[1]) * stride[n] - padh) * ratioh);
                        const int width = (int)((pdata_bbox[2] + pdata_bbox[0])*stride[n] * ratiow);
                        const int height = (int)((pdata_bbox[3] + pdata_bbox[1])*stride[n] * ratioh);
                        confidences.push_back(pdata_score[0]);
                        boxes.push_back(cv::Rect(xmin, ymin, width, height));
                        std::vector<int> landmark(10, 0);
                        for (l = 0; l < 10; l+=2)
                        {
                            landmark[l] = (int)(((j + pdata_kps[l]) * stride[n] - padw) * ratiow);
                            landmark[l + 1] = (int)(((i + pdata_kps[l + 1]) * stride[n] - padh) * ratioh);
                        }
                        landmarks.push_back(landmark);
                    }
                    pdata_score++;
                    pdata_bbox += 4;
                    pdata_kps += 10;
                }
            }
        }
    }

    // Perform non maximum suppression to eliminate redundant overlapping boxes with
    // lower confidences
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    std::vector<int> xs;
    std::vector<int> ys;
    std::vector<float> confs;

    for (i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];

        for (k = 0; k < 4; k += 2)
        {
            xs.push_back(landmarks[idx][k]);
            ys.push_back(landmarks[idx][k + 1]);
        }
        confs.push_back(confidences[idx]);
    }
    return std::make_tuple(xs, ys, confs);
}