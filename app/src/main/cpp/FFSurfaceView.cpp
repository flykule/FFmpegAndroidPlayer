//
// Created by castle on 4/9/19.
//
#include <jni.h>
#include <FFSurfaceView.h>
#include <android/log.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#ifdef __cplusplus
}
#endif
#define TAG "FFSurfaceView"

//int jniPrint(const char *fmt) {
//    return __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt);
//}
//
//int jniPrint(const char *fmt, const char *args...) {
//    return __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, args);
//}

JNIEXPORT void JNICALL Java_com_castle_ffmpeg_player_view_FFSurfaceView_render
        (JNIEnv *env, jobject jobject1, jstring sourceUrl, jobject surfaceHolder) {
    const char *nativeString = env->GetStringUTFChars(sourceUrl, nullptr);
    // use your string
    env->ReleaseStringUTFChars(sourceUrl, nativeString);
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "Start process url: %s", nativeString);

    av_register_all();
    AVFormatContext *pFormatCtx = nullptr;
    if (avformat_open_input(&pFormatCtx, nativeString, nullptr, nullptr) != 0) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "Open video file failed! Provided path: %s\n",
                            nativeString);
        return;
    } else{
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "Avformat open input success!");
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "Read stream info failed!\n");
        return;
    }
    av_dump_format(pFormatCtx, 0, nativeString, 0);
}


