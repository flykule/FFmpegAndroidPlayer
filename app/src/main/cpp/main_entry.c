//
// Created by castle on 4/11/19.
//
#include <stdlib.h>
//#include <jni.h>
#include <string.h>
#include <assert.h>
// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <libnative-lib/player.h>

// set the playing state for the asset audio player
JNIEXPORT void JNICALL
Java_com_castle_ffmpeg_player_view_FFSurfaceView_setPlayingState(
        JNIEnv *env, jobject clazz, jboolean isPlaying) {
    if (isPlaying) {
        StartPlay();
    } else {
        PausePlay();
    }
}

// create asset audio player
JNIEXPORT jboolean JNICALL
Java_com_castle_ffmpeg_player_view_FFSurfaceView_createPlayer(
        JNIEnv *env,
        jobject clazz,
        jobject surface,
        jstring filename) {
    // convert Java string to UTF-8
    const char *utf8 = (*env)->GetStringUTFChars(env, filename, NULL);
    assert(NULL != utf8);
    MediaInfo *mediaInfo = malloc(sizeof(MediaInfo));
    mediaInfo->uri = malloc(sizeof(char) * (strlen(utf8) + 1));
    memset(mediaInfo->uri, 0, strlen(utf8) + 1);
    strncpy(mediaInfo->uri, utf8, strlen(utf8) + 1);
    mediaInfo->uri[strlen(utf8)] = '\0';
    (*env)->ReleaseStringUTFChars(env, filename, utf8);
    int result = CreatePlayerInstance(env, surface, mediaInfo);
    if (result != 0) {
        LOGE("Error in create player instance!");
        ReleaseAll();
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

// create the engine and output mix objects
// shut down the native audio system
JNIEXPORT void JNICALL
Java_com_castle_ffmpeg_player_view_FFSurfaceView_shutdown(JNIEnv *env,
                                                          jobject clazz) {
//    StopPlay();
    ReleaseAll();
}

JNIEXPORT void JNICALL Java_com_castle_ffmpeg_player_view_FFSurfaceView_render
        (JNIEnv *env, jobject jobject1, jstring sourceUrl, jobject surfaceHolder) {
//    const char *nativeString = (*env)->GetStringUTFChars(env, sourceUrl, NULL);
//    // use your string
//    (*env)->ReleaseStringUTFChars(env, sourceUrl, nativeString);
//    LOGD("Start process url: %s", nativeString);
//
//    av_register_all();
//    AVFormatContext *pFormatCtx = NULL;
//    if (avformat_open_input(&pFormatCtx, nativeString, NULL, NULL) != 0) {
//        LOGE("Open video file failed! Provided path: %s\n", nativeString);
//        return;
//    } else {
//        LOGD("Avformat open input success!");
//    }
//    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
//        LOGE("Read stream info failed!\n");
//        return;
//    } else {
//        LOGD("Find stream info success!");
//    }
////    av_dump_format(pFormatCtx, 0, nativeString, 0);
//    AVCodec *videoCodec = NULL;
//    int videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &videoCodec, 0);
//    LOGD("Find video stream index: %d", videoStream);
//    AVCodec *audioCodec = NULL;
//    int audioStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &audioCodec, 0);
//    LOGD("Find audio stream index: %d", audioStream);
//    AVCodecContext *audioCtx = pFormatCtx->streams[audioStream]->codec;
//    if (avcodec_open2(audioCtx, audioCodec, NULL) < 0) {
//        LOGE("Open audioCodec failed!");
//        return;
//    } else {
//        LOGD("Open audioCodec success!");
//    }
//    AVCodecContext *avctx = NULL;
//    avctx = pFormatCtx->streams[videoStream]->codec;
//    if (avctx != NULL) {
//        LOGD("Get video videoCodec context success!");
//    } else {
//        LOGE("Get video videoCodec context failed!");
//        return;
//    }
//    if (avcodec_open2(avctx, videoCodec, NULL) < 0) {
//        LOGE("Open videoCodec failed!");
//        return;
//    } else {
//        LOGD("Open videoCodec success!");
//    }
//    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
//    av_init_packet(packet);
//    int out_nb_samples = avctx->frame_size;
//    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
//    int out_sample_rate = avctx->sample_rate;
//    int out_channels = avctx->channels;
//    //Out Buffer Size
//
//    AVFrame *pFrame = av_frame_alloc();
//    AVFrame *rgbFrame = av_frame_alloc();
////    uint8_t *audio_out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
//    //缓存区
//    uint8_t *out_buffer = (uint8_t *) av_malloc((size_t) avpicture_get_size(AV_PIX_FMT_RGBA,
//                                                                            avctx->width,
//                                                                            avctx->height));
//    //与缓存区相关联，设置rgb_frame缓存区
//    avpicture_fill((AVPicture *) rgbFrame, out_buffer, AV_PIX_FMT_RGBA, avctx->width,
//                   avctx->height);
//    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surfaceHolder);
//    if (nativeWindow == NULL) {
//        LOGE("Get native window failed!");
//        return;
//    } else {
//        LOGD("Get a native window success!");
//    }
//    struct SwsContext *swsContext = sws_getContext(avctx->width, avctx->height, avctx->pix_fmt,
//                                                   avctx->width, avctx->height, AV_PIX_FMT_RGBA,
//                                                   SWS_BILINEAR,
//                                                   NULL, NULL, NULL);
//
//    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
//    int64_t in_channel_layout = av_get_default_channel_layout(avctx->channels);
//    struct SwrContext *au_convert_ctx = swr_alloc_set_opts(NULL, (int64_t) out_channel_layout,
//                                                           out_sample_fmt, out_sample_rate,
//                                                           in_channel_layout, avctx->sample_fmt,
//                                                           avctx->sample_rate, 0, NULL);
//    swr_init(au_convert_ctx);
//
//    ANativeWindow_Buffer nativeWindow_buffer;
//    int frameCount;
//    LOGD("Avcodec attr width %d height %d", avctx->width, avctx->height);
//
//    //Start event loop
//    while (av_read_frame(pFormatCtx, packet) >= 0) {
//        if (packet->stream_index == audioStream) {
//            // 发送一个AVPacket(AAC)到解码器
//            int ret = avcodec_send_packet(audioCtx, packet);
//            if (ret != 0) {
//                return;
//            }
//            // 循环读取，获取一帧完整PCM音频数据
//            while (avcodec_receive_frame(audioCtx, pFrame) == 0) {
//                LOGD("读取一帧音频数据,frameSize=%d", pFrame->nb_samples);
//                break;
//            }
//            //Out Buffer Size
//            int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples,
//                                                             out_sample_fmt, 1);
//            if (internalbuffer != NULL) {
//                av_free(internalbuffer);
//                internalbuffer = NULL;
//            }
//            internalbuffer = av_malloc(sizeof(uint8_t) * out_buffer_size);
//            swr_convert(au_convert_ctx, &internalbuffer, out_buffer_size,
//                        (const uint8_t **) pFrame->data, pFrame->nb_samples);
////            memset(nextBuffer,0,out_buffer_size);
////            memcpy(nextBuffer,)
//            nextBuffer = internalbuffer;
//            nextSize = (uint) out_buffer_size;
////            bqPlayerCallback(bqPlayerBufferQueue,NULL);
////            bqPlayerCallback()
//            if (nextSize > 0) {
//                LOGD("Decode a frame of audio...");
//                // here we only enqueue one buffer because it is a long clip,
//                // but for streaming playback we would typically enqueue at least 2 buffers to start
//                SLresult result;
//                result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer,
//                                                         nextSize);
////                if (SL_RESULT_SUCCESS != result) {
////                    pthread_mutex_unlock(&audioEngineLock);
////                    LOGE("音频包入栈产生错误");
////                    return;
////                }
//            }
//        }
//        if (packet->stream_index == videoStream) {
//            int ret = avcodec_decode_video2(avctx, pFrame, &frameCount, packet);
//            if (ret < 0) {
//                LOGE("Error in decoding video frame.");
//                return;
//            }
//            if (frameCount) {
////                LOGE("Draw frame");
//                //说明有内容
//                //绘制之前配置nativewindow
//                ANativeWindow_setBuffersGeometry(nativeWindow, avctx->width, avctx->height,
//                                                 WINDOW_FORMAT_RGBA_8888);
////                LOGE("Window buffer attr: width %d height %d  stride %d", nativeWindow_buffer.width,
////                     nativeWindow_buffer.height, nativeWindow_buffer.stride);
//                //上锁
//                ANativeWindow_lock(nativeWindow, &nativeWindow_buffer, NULL);
////                LOGE("Check everything ok...")
//                //转换为rgb格式
////                LOGE("Check everything: %d %d", pFrame->height, rgbFrame->height);
//
//                sws_scale(swsContext, (const uint8_t *const *) pFrame->data, pFrame->linesize,
//                          0,
//                          avctx->height, rgbFrame->data,
//                          rgbFrame->linesize);
////                LOGE("This step ok");
//                //  rgb_frame是有画面数据
//                uint8_t *dst = (uint8_t *) nativeWindow_buffer.bits;
////            拿到一行有多少个字节 RGBA
//                int destStride = nativeWindow_buffer.stride * 4;
//                //像素数据的首地址
//                uint8_t *src = rgbFrame->data[0];
////            实际内存一行数量
//                int srcStride = rgbFrame->linesize[0];
//                for (int h = 0; h < avctx->height; h++) {
//                    memcpy(dst + h * destStride,
//                           src + h * srcStride,
//                           (size_t) destStride);
//                }
////解锁
//                ANativeWindow_unlockAndPost(nativeWindow);
//                usleep(1000 * 16);
//            }
//        }
//        av_packet_unref(packet);
//        av_frame_unref(pFrame);
////        av_free_packet(packet);
//    }
//    ANativeWindow_release(nativeWindow);
//    av_frame_free(&pFrame);
//    av_frame_free(&rgbFrame);
//    avcodec_close(avctx);
//    avformat_free_context(pFormatCtx);
////    env->ReleaseStringUTFChars(sourceUrl, nativeString);
}

