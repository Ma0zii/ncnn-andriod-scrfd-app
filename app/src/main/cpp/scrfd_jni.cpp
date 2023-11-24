//
// Created by maozii on 2023/11/10.
//
#include <jni.h>
#include <string>
#include <net.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/log.h>
#include "scrfd.hpp"

static ncnn::Net scrfd;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_retainfacencnn_SCRFD_Init(JNIEnv *env, jobject thiz, jobject assetManager) {
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    //init param
    int ret = scrfd.load_param(mgr, "scrfd_2.5g_kps-opt2.param");
    if (ret != 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "scrfd", "load_param failed");
        return JNI_FALSE;
    }
    //init bin
    ret = scrfd.load_model(mgr, "scrfd_2.5g_kps-opt2.bin");
    if (ret != 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "scrfd", "load_model failed");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}


extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_example_retainfacencnn_SCRFD_Detect(JNIEnv *env, jobject thiz, jobject bitmap) {

    ncnn::Extractor ex = scrfd.create_extractor();

    ncnn::Mat in = ncnn::Mat::from_android_bitmap(env, bitmap, ncnn::Mat::PIXEL_BGR);
    std::vector<FaceObject> objs = detect_scrfd(scrfd, in);
    int count = static_cast<int>(objs.size()), ix = 0;
    if (count <= 0)
        return nullptr;
    //result to 1D-array
    count = static_cast<int>(count * 15);
    float *face_info = new float[count];
    for (auto obj : objs) {
        face_info[ix++] = obj.rect.x;
        face_info[ix++] = obj.rect.y;
        face_info[ix++] = obj.rect.x + obj.rect.width;
        face_info[ix++] = obj.rect.y + obj.rect.height;
        for (int j = 0; j < 5; j++) {
            face_info[ix++] = obj.landmark[j].x;
            face_info[ix++] = obj.landmark[j].y;
        }
        face_info[ix++] = obj.prob;
    }
    jfloatArray tFaceInfo = env->NewFloatArray(count);
    env->SetFloatArrayRegion(tFaceInfo, 0, count, face_info);
    delete[] face_info;
    return tFaceInfo;
}


