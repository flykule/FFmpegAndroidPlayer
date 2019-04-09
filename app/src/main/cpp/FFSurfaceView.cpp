//
// Created by castle on 4/9/19.
//
#include <jni.h>
#include <FFSurfaceView.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

JNIEXPORT void JNICALL Java_com_castle_ffmpeg_player_view_FFSurfaceView_render
        (JNIEnv *env, jobject jobject1, jstring sourceUrl, jobject surfaceHolder) {
    const char *nativeString = env->GetStringUTFChars(sourceUrl, nullptr);
    // use your string
    env->ReleaseStringUTFChars(sourceUrl, nativeString);

    av_register_all();
    AVFormatContext *pFormatCtx = nullptr;
    if (avformat_open_input(&pFormatCtx, nativeString, nullptr, nullptr) != 0) {
        printf("Open video file failed! Provided path: %s\n", nativeString);
        return;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        printf("Read stream info failed!\n");
        return;
    }
    av_dump_format(pFormatCtx, 0, nativeString, 0);
}

