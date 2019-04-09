//
// Created by castle on 4/9/19.
//
#include <jni.h>
#include <android/log.h>
#include <libnative-lib/FFSurfaceView.h>
//#include <android/>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>
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
    SwsContext *swsContext = sws_getContext(avctx->width, avctx->height, avctx->pix_fmt,
                                            avctx->width, avctx->height, AV_PIX_FMT_RGBA,
                                            SWS_BILINEAR,
                                            nullptr, nullptr, nullptr);
    ANativeWindow_Buffer nativeWindow_buffer;
    int frameCount;
    LOGD("Avcodec attr width %d height %d", avctx->width, avctx->height);
    //Start event loop
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == videoStream) {
            avcodec_decode_video2(avctx, pFrame, &frameCount, packet);
            if (frameCount) {
                LOGE("Draw frame");
                //说明有内容
                //绘制之前配置nativewindow
                ANativeWindow_setBuffersGeometry(nativeWindow, avctx->width, avctx->height,
                                                 WINDOW_FORMAT_RGBA_8888);
                LOGE("Window buffer attr: width %d height %d  stride %d", nativeWindow_buffer.width,
                     nativeWindow_buffer.height, nativeWindow_buffer.stride);
                //上锁
                ANativeWindow_lock(nativeWindow, &nativeWindow_buffer, NULL);
//                LOGE("Check everything ok...")
                //转换为rgb格式
                LOGE("Check everything: %d %d", pFrame->height, rgbFrame->height);

                sws_scale(swsContext, (const uint8_t *const *) pFrame->data, pFrame->linesize, 0,
                          avctx->height, rgbFrame->data,
                          rgbFrame->linesize);
                LOGE("This step ok");
                //  rgb_frame是有画面数据
                uint8_t *dst = (uint8_t *) nativeWindow_buffer.bits;
//            拿到一行有多少个字节 RGBA
                int destStride = nativeWindow_buffer.stride * 4;
                //像素数据的首地址
                uint8_t *src = rgbFrame->data[0];
//            实际内存一行数量
                int srcStride = rgbFrame->linesize[0];
                for (int h = 0; h < avctx->height; h++) {
                    memcpy(dst + h * destStride,
                           src + h * srcStride,
                           (size_t) destStride);
                }
//解锁
                ANativeWindow_unlockAndPost(nativeWindow);
                usleep(1000 * 16);
            }
//            int ret = avcode
        }
//        av_packet_unref(packet);
        av_free_packet(packet);
    }
    ANativeWindow_release(nativeWindow);
    av_frame_free(&pFrame);
    av_frame_free(&rgbFrame);
    avcodec_close(avctx);
    avformat_free_context(pFormatCtx);
    env->ReleaseStringUTFChars(sourceUrl, nativeString);
}


