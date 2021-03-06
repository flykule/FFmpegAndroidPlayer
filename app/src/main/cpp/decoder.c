//
// Created by castle on 4/16/19.
//
#include <libnative-lib/decoder.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libnative-lib/util.h>

static AVFormatContext *pFormatContext = NULL;
static AVCodec *pVideoCodec = NULL;
static AVCodec *pAudioCodec = NULL;
static AVCodecContext *pAudioCodecContext = NULL;
static AVCodecContext *pVideoCodecContext = NULL;
static AVCodecParameters *pAudioCodecParameters = NULL;
static AVCodecParameters *pVideoCodecParameters = NULL;
static int audioStream = -1;
static int videoStream = -1;
static SwrContext *pSwrContext = NULL;
static AVPacket *pPacket = NULL;
static AVFrame *pFrame = NULL;
static AVFrame *rgbFrame = NULL;
static struct SwsContext *pSwsContext = NULL;
static uint8_t *internal_buffer = NULL;
static ANativeWindow *pWindow = NULL;
static ANativeWindow_Buffer nativeWindow_buffer;

void ReleaseResources(void) {
    audioStream = -1;
    videoStream = -1;
    if (pFrame != NULL) {
        av_frame_unref(pFrame);
        pFrame = NULL;
    }
    if (pWindow != NULL) {
        ANativeWindow_release(pWindow);
        pWindow = NULL;
    }
    if (pSwsContext != NULL) {
        sws_freeContext(pSwsContext);
        pSwsContext = NULL;
    }
    if (pPacket != NULL) {
        av_packet_unref(pPacket);
        pPacket = NULL;
    }
    if (internal_buffer != NULL) {
        av_free(internal_buffer);
        internal_buffer = NULL;
    }
    if (pSwrContext != NULL) {
        swr_free(&pSwrContext);
        pSwrContext = NULL;
    }
    if (pVideoCodecParameters != NULL) {
        pVideoCodecParameters = NULL;
    }
    if (pVideoCodecContext != NULL) {
        avcodec_free_context(&pVideoCodecContext);
        pVideoCodecContext = NULL;
    }
    if (pAudioCodecParameters != NULL) {
        pAudioCodecParameters = NULL;
    }
    if (pAudioCodecContext != NULL) {
        avcodec_free_context(&pAudioCodecContext);
        pAudioCodecContext = NULL;
    }
    avformat_close_input(&pFormatContext);
    if (pFormatContext != NULL) {
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }
}

int CreateDecoder(const char *filepath, VideoInfo *infos) {
    av_register_all();
    char errMsg[1024];
    if (avformat_open_input(&pFormatContext, filepath, NULL, NULL) != 0) {
        sprintf(errMsg, "Open video file failed! Provided path: %s\n", filepath);
        goto RELEASE;
    }
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        sprintf(errMsg, "Read stream info failed!\n");
        goto RELEASE;
    }
    videoStream = av_find_best_stream(pFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &pVideoCodec, 0);
    audioStream = av_find_best_stream(pFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &pAudioCodec, 0);
    if (audioStream == -1 || videoStream == -1) {
        sprintf(errMsg,
                "One of audio/video stream index is invalid! AudioStream index: %d Video stream index: %d",
                audioStream, videoStream);
        goto RELEASE;
    }
    if (pVideoCodec == NULL || pAudioCodec == NULL) {
        sprintf(errMsg, "One of audio/video codec is invalid!");
        goto RELEASE;
    }
    pAudioCodecParameters = pFormatContext->streams[audioStream]->codecpar;
    pVideoCodecParameters = pFormatContext->streams[videoStream]->codecpar;
    if (pAudioCodecParameters == NULL || pVideoCodecParameters == NULL) {
        sprintf(errMsg, "One of audio/video codec parameters is invalid!");
        goto RELEASE;
    }
    pAudioCodecContext = avcodec_alloc_context3(pAudioCodec);
    pVideoCodecContext = avcodec_alloc_context3(pVideoCodec);
    avcodec_parameters_to_context(pAudioCodecContext, pAudioCodecParameters);
    avcodec_parameters_to_context(pVideoCodecContext, pVideoCodecParameters);
    if (avcodec_open2(pAudioCodecContext, pAudioCodec, NULL) < 0 ||
        avcodec_open2(pVideoCodecContext, pVideoCodec, NULL) < 0) {
        sprintf(errMsg, "Audio/Video codec context open failed!");
        goto RELEASE;
    }
    pSwrContext = swr_alloc();
    pSwrContext = swr_alloc_set_opts(pSwrContext, (int64_t) pAudioCodecContext->channel_layout,
                                     AV_SAMPLE_FMT_S16, pAudioCodecContext->sample_rate,
                                     pAudioCodecContext->channel_layout,
                                     pAudioCodecContext->sample_fmt,
                                     pAudioCodecContext->sample_rate, 0, NULL);
    swr_init(pSwrContext);
    infos->channels = pAudioCodecContext->channels;
    infos->duration = (pFormatContext->duration * 1000) / AV_TIME_BASE;
    infos->samplerate = pAudioCodecContext->sample_rate;
    return 0;
    RELEASE:
    if (strlen(errMsg) > 0) {
        LOGE("%s", errMsg);
        free(errMsg);
    }
    ReleaseResources();
    return -1;
}

int CreateNativeWindow(JNIEnv *env, jobject surface) {
    if (!env || !surface) {
        return -1;
    }
    pWindow = ANativeWindow_fromSurface(env, surface);
    if (!pWindow) {
        return -1;
    }
    rgbFrame = av_frame_alloc();
    //缓存区
    uint8_t *out_buffer = (uint8_t *) av_malloc((size_t) avpicture_get_size(AV_PIX_FMT_RGBA,
                                                                            pVideoCodecContext->width,
                                                                            pVideoCodecContext->height));
    //与缓存区相关联，设置rgb_frame缓存区
    avpicture_fill((AVPicture *) rgbFrame, out_buffer, AV_PIX_FMT_RGBA, pVideoCodecContext->width,
                   pVideoCodecContext->height);

    pSwsContext = sws_getContext(pVideoCodecContext->width, pVideoCodecContext->height,
                                 pVideoCodecContext->pix_fmt,
                                 pVideoCodecContext->width, pVideoCodecContext->height,
                                 AV_PIX_FMT_RGBA,
                                 SWS_BILINEAR,
                                 NULL, NULL, NULL);
    return pSwrContext == NULL ? -1 : 0;
}

/**
 * Main loop
 */
int getAudioSource(void **buffer, size_t *buffersize) {
    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        if (pPacket->stream_index == audioStream) {
            int ret = avcodec_send_packet(pAudioCodecContext, pPacket);
            if (ret < 0) {
                return -1;
            }
//            // 循环读取，获取一帧完整PCM音频数据
            ret = avcodec_receive_frame(pAudioCodecContext, pFrame);
            if (ret == AVERROR(EAGAIN)) {
                //Not ready yet
                continue;
            }
            if (ret < 0) {
                return -1;
            }
            //Out Buffer Size
            int out_buffer_size = av_samples_get_buffer_size(NULL, pFrame->channels,
                                                             pFrame->nb_samples,
                                                             AV_SAMPLE_FMT_S16, 1);
            if (internal_buffer != NULL) {
                av_free(internal_buffer);
                internal_buffer = NULL;
            }
            internal_buffer = av_malloc(sizeof(uint8_t) * out_buffer_size);
            swr_convert(pSwrContext, &internal_buffer, out_buffer_size,
                        (const uint8_t **) pFrame->data, pFrame->nb_samples);
            *buffer = internal_buffer;
            *buffersize = (uint) out_buffer_size;
            av_packet_unref(pPacket);
            av_frame_unref(pFrame);
            pPacket = NULL;
            pFrame = NULL;
            return 0;
        }
        if (pPacket->stream_index == videoStream) {
            int ret = avcodec_send_packet(pVideoCodecContext, pPacket);
            if (ret < 0) {
                return -1;
            }
//            // 循环读取，获取一帧完整PCM音频数据
            ret = avcodec_receive_frame(pVideoCodecContext, pFrame);
            if (ret == AVERROR(EAGAIN)) {
                //Not ready yet
                continue;
            }
            if (ret < 0) {
                return -1;
            }
            ANativeWindow_setBuffersGeometry(pWindow, pVideoCodecContext->width,
                                             pVideoCodecContext->height,
                                             WINDOW_FORMAT_RGBA_8888);
            //上锁
            ANativeWindow_lock(pWindow, &nativeWindow_buffer, NULL);
            //转换为rgb格式
            sws_scale(pSwsContext, (const uint8_t *const *) pFrame->data, pFrame->linesize,
                      0,
                      pVideoCodecContext->height, rgbFrame->data,
                      rgbFrame->linesize);
//                LOGE("This step ok");
            //  rgb_frame是有画面数据
            uint8_t *dst = (uint8_t *) nativeWindow_buffer.bits;
//            拿到一行有多少个字节 RGBA
            int destStride = nativeWindow_buffer.stride * 4;
            //像素数据的首地址
            uint8_t *src = rgbFrame->data[0];
//            实际内存一行数量
            int srcStride = rgbFrame->linesize[0];
            for (int h = 0; h < pVideoCodecContext->height; h++) {
                memcpy(dst + h * destStride,
                       src + h * srcStride,
                       (size_t) destStride);
            }
//解锁
            ANativeWindow_unlockAndPost(pWindow);
            av_packet_unref(pPacket);
            av_frame_unref(pFrame);
//            pPacket = NULL;
//            pFrame = NULL;
//            return 0;
//            usleep(1000 * 16);
        }
    }
    return -1;
}

