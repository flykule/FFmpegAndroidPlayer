//
// Created by castle on 4/11/19.
//
#include <stdlib.h>
#include <jni.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// file descriptor player interfaces
static SLObjectItf fdPlayerObject = NULL;
static SLPlayItf fdPlayerPlay;
static SLSeekItf fdPlayerSeek;
static SLMuteSoloItf fdPlayerMuteSolo;
static SLVolumeItf fdPlayerVolume;

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>
#include <libnative-lib/util.h>
#include <libswresample/swresample.h>


// pointer and size of the next player buffer to enqueue, and number of remaining buffers
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
//uint8_t* out_buffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
static void *nextBuffer;
static unsigned nextSize;
static int nextCount;
static uint8_t *internalbuffer = NULL;

// a mutext to guard against re-entrance to record & playback
// as well as make recording and playing back to be mutually exclusive
// this is to avoid crash at situations like:
//    recording is in session [not finished]
//    user presses record button and another recording coming in
// The action: when recording/playing back is not finished, ignore the new request
static pthread_mutex_t audioEngineLock = PTHREAD_MUTEX_INITIALIZER;

// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
        SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLEffectSendItf bqPlayerEffectSend;
static SLMuteSoloItf bqPlayerMuteSolo;
static SLVolumeItf bqPlayerVolume;
static SLmilliHertz bqPlayerSampleRate = 0;
static jint bqPlayerBufSize = 0;
static short *resampleBuf = NULL;

void releaseResampleBuf(void) {
    if (0 == bqPlayerSampleRate) {
        /*
         * we are not using fast path, so we were not creating buffers, nothing to do
         */
        return;
    }

    free(resampleBuf);
    resampleBuf = NULL;
}
// set the playing state for the asset audio player
JNIEXPORT void JNICALL
Java_com_castle_ffmpeg_player_activity_NativeAudioActivity_00024Companion_setPlayingAssetAudioPlayer(
        JNIEnv *env,
        jobject clazz,
        jboolean isPlaying) {
    SLresult result;

    // make sure the asset audio player was created
    if (NULL != fdPlayerPlay) {

        // set the player's state
        result = (*fdPlayerPlay)->SetPlayState(fdPlayerPlay, isPlaying ?
                                                             SL_PLAYSTATE_PLAYING
                                                                       : SL_PLAYSTATE_PAUSED);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;
    }

}

// create asset audio player
JNIEXPORT jboolean JNICALL
Java_com_castle_ffmpeg_player_activity_NativeAudioActivity_00024Companion_createAssetAudioPlayer(
        JNIEnv *env,
        jobject clazz,
        jobject assetManager,
        jstring filename) {
    SLresult result;

    // convert Java string to UTF-8
    const char *utf8 = (*env)->GetStringUTFChars(env, filename, NULL);
    assert(NULL != utf8);

    // use asset manager to open asset by filename
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    assert(NULL != mgr);
    AAsset *asset = AAssetManager_open(mgr, utf8, AASSET_MODE_UNKNOWN);

    // release the Java string and UTF-8
    (*env)->ReleaseStringUTFChars(env, filename, utf8);

    // the asset might not be found
    if (NULL == asset) {
        return JNI_FALSE;
    }

    // open asset as file descriptor
    off_t start, length;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
    assert(0 <= fd);
    AAsset_close(asset);

    // configure audio source
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSrc = {&loc_fd, &format_mime};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &fdPlayerObject, &audioSrc, &audioSnk,
                                                3, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the player
    result = (*fdPlayerObject)->Realize(fdPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the play interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_PLAY, &fdPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the seek interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_SEEK, &fdPlayerSeek);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the mute/solo interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_MUTESOLO, &fdPlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the volume interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_VOLUME, &fdPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // enable whole file looping
    result = (*fdPlayerSeek)->SetLoop(fdPlayerSeek, SL_BOOLEAN_TRUE, 0, SL_TIME_UNKNOWN);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    return JNI_TRUE;
}

// create the engine and output mix objects
JNIEXPORT void JNICALL
Java_com_castle_ffmpeg_player_activity_NativeAudioActivity_00024Companion_createEngine(JNIEnv *env,
                                                                                       jobject clazz) {
    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }
    // ignore unsuccessful result codes for environmental reverb, as it is optional for this example

}

// shut down the native audio system
JNIEXPORT void JNICALL
Java_com_castle_ffmpeg_player_activity_NativeAudioActivity_00024Companion_shutdown(JNIEnv *env,
                                                                                   jobject clazz) {

    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerEffectSend = NULL;
        bqPlayerMuteSolo = NULL;
        bqPlayerVolume = NULL;
    }

    // destroy file descriptor audio player object, and invalidate all associated interfaces
    if (fdPlayerObject != NULL) {
        (*fdPlayerObject)->Destroy(fdPlayerObject);
        fdPlayerObject = NULL;
        fdPlayerPlay = NULL;
        fdPlayerSeek = NULL;
        fdPlayerMuteSolo = NULL;
        fdPlayerVolume = NULL;
    }

    // destroy URI audio player object, and invalidate all associated interfaces
//    if (uriPlayerObject != NULL) {
//        (*uriPlayerObject)->Destroy(uriPlayerObject);
//        uriPlayerObject = NULL;
//        uriPlayerPlay = NULL;
//        uriPlayerSeek = NULL;
//        uriPlayerMuteSolo = NULL;
//        uriPlayerVolume = NULL;
//    }

    // destroy audio recorder object, and invalidate all associated interfaces
//    if (recorderObject != NULL) {
//        (*recorderObject)->Destroy(recorderObject);
//        recorderObject = NULL;
//        recorderRecord = NULL;
//        recorderBufferQueue = NULL;
//    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    pthread_mutex_destroy(&audioEngineLock);
}


// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    assert(bq == bqPlayerBufferQueue);
    assert(NULL == context);
    // for streaming playback, replace this test by logic to find and fill the next buffer
    if (NULL != nextBuffer && 0 != nextSize) {
        SLresult result;
        // enqueue another buffer
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, nextSize);
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
        if (SL_RESULT_SUCCESS != result) {
            pthread_mutex_unlock(&audioEngineLock);
        }
        (void) result;
    } else {
        releaseResampleBuf();
        pthread_mutex_unlock(&audioEngineLock);
    }
}

// create buffer queue audio player
JNIEXPORT void JNICALL
Java_com_castle_ffmpeg_player_activity_NativeAudioActivity_00024Companion_createBufferQueueAudioPlayer(
        JNIEnv *env,
        jobject clazz, jint sampleRate,
        jint bufSize) {
    SLresult result;
    if (sampleRate >= 0 && bufSize >= 0) {
        bqPlayerSampleRate = sampleRate * 1000;
        /*
         * device native buffer size is another factor to minimize audio latency, not used in this
         * sample: we only play one giant buffer here
         */
        bqPlayerBufSize = bufSize;
    }

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_48,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    /*
     * Enable Fast Audio when possible:  once we set the same rate to be the native, fast audio path
     * will be triggered
     */
    if (bqPlayerSampleRate) {
        format_pcm.samplesPerSec = bqPlayerSampleRate;       //sample rate in mili second
    }
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ };

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                bqPlayerSampleRate ? 2 : 3, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the effect send interface
    bqPlayerEffectSend = NULL;
    if (0 == bqPlayerSampleRate) {
        result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
                                                 &bqPlayerEffectSend);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;
    }

#if 0   // mute/solo is not supported for sources that are known to be mono, as this is
    // get the mute/solo interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
#endif

    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
}


JNIEXPORT void JNICALL Java_com_castle_ffmpeg_player_view_FFSurfaceView_render
        (JNIEnv *env, jobject jobject1, jstring sourceUrl, jobject surfaceHolder) {
    const char *nativeString = (*env)->GetStringUTFChars(env, sourceUrl, NULL);
    // use your string
    (*env)->ReleaseStringUTFChars(env, sourceUrl, nativeString);
    LOGD("Start process url: %s", nativeString);

    av_register_all();
    AVFormatContext *pFormatCtx = NULL;
    if (avformat_open_input(&pFormatCtx, nativeString, NULL, NULL) != 0) {
        LOGE("Open video file failed! Provided path: %s\n", nativeString);
        return;
    } else {
        LOGD("Avformat open input success!");
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("Read stream info failed!\n");
        return;
    } else {
        LOGD("Find stream info success!");
    }
//    av_dump_format(pFormatCtx, 0, nativeString, 0);
    AVCodec *videoCodec = NULL;
    int videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &videoCodec, 0);
    LOGD("Find video stream index: %d", videoStream);
    AVCodec *audioCodec = NULL;
    int audioStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &audioCodec, 0);
    LOGD("Find audio stream index: %d", audioStream);
    AVCodecContext *audioCtx = pFormatCtx->streams[audioStream]->codec;
    if (avcodec_open2(audioCtx, audioCodec, NULL) < 0) {
        LOGE("Open audioCodec failed!");
        return;
    } else {
        LOGD("Open audioCodec success!");
    }
    AVCodecContext *avctx = NULL;
    avctx = pFormatCtx->streams[videoStream]->codec;
    if (avctx != NULL) {
        LOGD("Get video videoCodec context success!");
    } else {
        LOGE("Get video videoCodec context failed!");
        return;
    }
    if (avcodec_open2(avctx, videoCodec, NULL) < 0) {
        LOGE("Open videoCodec failed!");
        return;
    } else {
        LOGD("Open videoCodec success!");
    }
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    int out_nb_samples = avctx->frame_size;
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = avctx->sample_rate;
    int out_channels = avctx->channels;
    //Out Buffer Size

    AVFrame *pFrame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();
//    uint8_t *audio_out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
    //缓存区
    uint8_t *out_buffer = (uint8_t *) av_malloc((size_t) avpicture_get_size(AV_PIX_FMT_RGBA,
                                                                            avctx->width,
                                                                            avctx->height));
    //与缓存区相关联，设置rgb_frame缓存区
    avpicture_fill((AVPicture *) rgbFrame, out_buffer, AV_PIX_FMT_RGBA, avctx->width,
                   avctx->height);
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surfaceHolder);
    if (nativeWindow == NULL) {
        LOGE("Get native window failed!");
        return;
    } else {
        LOGD("Get a native window success!");
    }
    struct SwsContext *swsContext = sws_getContext(avctx->width, avctx->height, avctx->pix_fmt,
                                                   avctx->width, avctx->height, AV_PIX_FMT_RGBA,
                                                   SWS_BILINEAR,
                                                   NULL, NULL, NULL);

    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    int64_t in_channel_layout = av_get_default_channel_layout(avctx->channels);
    struct SwrContext *au_convert_ctx = swr_alloc_set_opts(NULL, (int64_t) out_channel_layout,
                                                           out_sample_fmt, out_sample_rate,
                                                           in_channel_layout, avctx->sample_fmt,
                                                           avctx->sample_rate, 0, NULL);
    swr_init(au_convert_ctx);

    ANativeWindow_Buffer nativeWindow_buffer;
    int frameCount;
    LOGD("Avcodec attr width %d height %d", avctx->width, avctx->height);

    //Start event loop
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == audioStream) {
            // 发送一个AVPacket(AAC)到解码器
            int ret = avcodec_send_packet(audioCtx, packet);
            if (ret != 0) {
                return;
            }
            // 循环读取，获取一帧完整PCM音频数据
            while (avcodec_receive_frame(audioCtx, pFrame) == 0) {
                LOGD("读取一帧音频数据,frameSize=%d", pFrame->nb_samples);
                break;
            }
            //Out Buffer Size
            int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples,
                                                             out_sample_fmt, 1);
            if (internalbuffer != NULL) {
                av_free(internalbuffer);
                internalbuffer = NULL;
            }
            internalbuffer = av_malloc(sizeof(uint8_t) * out_buffer_size);
            swr_convert(au_convert_ctx, &internalbuffer, out_buffer_size,
                        (const uint8_t **) pFrame->data, pFrame->nb_samples);
//            memset(nextBuffer,0,out_buffer_size);
//            memcpy(nextBuffer,)
            nextBuffer = internalbuffer;
            nextSize = (uint) out_buffer_size;
//            bqPlayerCallback(bqPlayerBufferQueue,NULL);
//            bqPlayerCallback()
            if (nextSize > 0) {
                LOGD("Decode a frame of audio...");
                // here we only enqueue one buffer because it is a long clip,
                // but for streaming playback we would typically enqueue at least 2 buffers to start
                SLresult result;
                result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer,
                                                         nextSize);
//                if (SL_RESULT_SUCCESS != result) {
//                    pthread_mutex_unlock(&audioEngineLock);
//                    LOGE("音频包入栈产生错误");
//                    return;
//                }
            }
        }
        if (packet->stream_index == videoStream) {
            int ret = avcodec_decode_video2(avctx, pFrame, &frameCount, packet);
            if (ret < 0) {
                LOGE("Error in decoding video frame.");
                return;
            }
            if (frameCount) {
//                LOGE("Draw frame");
                //说明有内容
                //绘制之前配置nativewindow
                ANativeWindow_setBuffersGeometry(nativeWindow, avctx->width, avctx->height,
                                                 WINDOW_FORMAT_RGBA_8888);
//                LOGE("Window buffer attr: width %d height %d  stride %d", nativeWindow_buffer.width,
//                     nativeWindow_buffer.height, nativeWindow_buffer.stride);
                //上锁
                ANativeWindow_lock(nativeWindow, &nativeWindow_buffer, NULL);
//                LOGE("Check everything ok...")
                //转换为rgb格式
//                LOGE("Check everything: %d %d", pFrame->height, rgbFrame->height);

                sws_scale(swsContext, (const uint8_t *const *) pFrame->data, pFrame->linesize,
                          0,
                          avctx->height, rgbFrame->data,
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
                for (int h = 0; h < avctx->height; h++) {
                    memcpy(dst + h * destStride,
                           src + h * srcStride,
                           (size_t) destStride);
                }
//解锁
                ANativeWindow_unlockAndPost(nativeWindow);
                usleep(1000 * 16);
            }
        }
        av_packet_unref(packet);
        av_frame_unref(pFrame);
//        av_free_packet(packet);
    }
    ANativeWindow_release(nativeWindow);
    av_frame_free(&pFrame);
    av_frame_free(&rgbFrame);
    avcodec_close(avctx);
    avformat_free_context(pFormatCtx);
//    env->ReleaseStringUTFChars(sourceUrl, nativeString);
}

