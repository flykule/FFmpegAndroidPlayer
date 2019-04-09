//
// Created by castle on 4/9/19.
//
#include <jni.h>
#include <libnative-lib/FFSurfaceView.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
//#include <android/>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
}
#endif
#define TAG "FFSurfaceView.cpp"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

JNIEXPORT void JNICALL Java_com_castle_ffmpeg_player_view_FFSurfaceView_render
        (JNIEnv *env, jobject jobject1, jstring sourceUrl, jobject surfaceHolder) {
    const char *nativeString = env->GetStringUTFChars(sourceUrl, nullptr);
    // use your string
    env->ReleaseStringUTFChars(sourceUrl, nativeString);
    LOGD("Start process url: %s", nativeString);

    av_register_all();
    AVFormatContext *pFormatCtx = nullptr;
    if (avformat_open_input(&pFormatCtx, nativeString, nullptr, nullptr) != 0) {
        LOGE("Open video file failed! Provided path: %s\n", nativeString);
        return;
    } else {
        LOGD("Avformat open input success!");
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        LOGE("Read stream info failed!\n");
        return;
    } else {
        LOGD("Find stream info success!");
    }
//    av_dump_format(pFormatCtx, 0, nativeString, 0);
    AVCodec *codec = nullptr;
    int videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    LOGD("Find video stream index: %d", videoStream);
    AVCodecContext *avctx = nullptr;
    avctx = pFormatCtx->streams[videoStream]->codec;
    if (avctx != nullptr) {
        LOGD("Get video codec context success!");
    } else {
        LOGE("Get video codec context failed!");
        return;
    }
    if (avcodec_open2(avctx, codec, nullptr) < 0) {
        LOGE("Open codec failed!");
        return;
    } else {
        LOGD("Open codec success!");
    }
    auto *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();
    //缓存区
    uint8_t *out_buffer = (uint8_t *) av_malloc((size_t) avpicture_get_size(AV_PIX_FMT_RGBA,
                                                                            avctx->width,
                                                                            avctx->height));
    //与缓存区相关联，设置rgb_frame缓存区
    avpicture_fill((AVPicture *) rgbFrame, out_buffer, AV_PIX_FMT_RGBA, avctx->width,
                   avctx->height);
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surfaceHolder);
    if (nativeWindow == nullptr) {
        LOGE("Get native window failed!");
        return;
    } else {
        LOGD("Get a native window success!");
    }
//    ANativeWindow_Buffer nativeWindow_buffer;
    //Start event loop
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == videoStream) {
//            int ret = avcode
        }
    }
}


