#include "dnn_scr.hpp"
#include <jni.h>
#include <string>
#include <net.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

static cv::dnn::Net net;

extern "C" JNIEXPORT jboolean JNICALL Java_com_example_retainfacencnn_SCRFDDNN_Init(JNIEnv *env, jobject thiz, jobject assetManager)
{
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);

//    std::string modelPath = "scrfd_500m_kps.onnx";
//    net = cv::dnn::readNet(modelPath);

    net = cv::dnn::readNet("scrfd_500m_kps.onnx");

    if(net.empty())
    {
        __android_log_print(ANDROID_LOG_DEBUG, "model", "load model failed");
        return JNI_FALSE;
    }

    return JNI_TRUE;
}


extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_example_retainfacencnn_SCRFDDNN_Detect(JNIEnv *env, jobject thiz, jobject bitmap)
{
    AndroidBitmapInfo info;
    AndroidBitmap_getInfo(env, bitmap, &info);

    void * pixels;
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    int ih = info.height;
    int iw = info.width;

    cv::Mat mat(info.height, info.width, CV_8UC3, pixels);
    AndroidBitmap_unlockPixels(env, bitmap);

    cv::Mat in;
    int padh;
    int padw;

    std::tie(in, padh, padw) = resizeNorm(mat);

    auto result = detect_dnn(net, in, ih, iw, padh, padw);
    std::vector<int> xs = std::get<0>(result);
    std::vector<int> ys = std::get<1>(result);
    std::vector<float> confs = std::get<2>(result);

    int count = static_cast<int>(confs.size());
    int ix = 0;

    if(count <= 0) return nullptr;
    count = static_cast<int>(count * 5);
    float *face_info = new float[count];

    for(int i = 0, j = 0; i < confs.size(); i++, j += 2)
    {
        face_info[ix++] = xs[j];
        face_info[ix++] = ys[j];
        face_info[ix++] = xs[j + 1];
        face_info[ix++] = ys[j + 1];
        face_info[ix++] = confs[i];
    }

    jfloatArray tFaceInfo = env->NewFloatArray(count);
    env->SetFloatArrayRegion(tFaceInfo, 0, count, face_info);
    delete[] face_info;

    return tFaceInfo;
}



