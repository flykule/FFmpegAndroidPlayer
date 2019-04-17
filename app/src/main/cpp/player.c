//
// Created by castle on 4/17/19.
//
#include <libnative-lib/player.h>
#include <libnative-lib/playerstate.h>
#include <libnative-lib/decoder.h>
#include <stdio.h>
#include <string.h>

#define EXITFUN                                                                \
  if (result != SL_RESULT_SUCCESS) {                                           \
    ReleasePlayer();                                                           \
    return -1;                                                                 \
  } else {                                                                       \
    (void)result;                                                             \
  }

#define PLAYEXIT                                                               \
  if (playerstate != OPENSLES_PLAYERSTATE_PREPARED) {                          \
    return;                                                                    \
  }

static SLObjectItf engineObject = NULL;
static SLObjectItf outputMixObject = NULL;
static SLObjectItf playerobj = NULL;
static SLEngineItf engineEngine = NULL;
static SLEnvironmentalReverbItf envrev = NULL;
static SLPlayItf player = NULL;
static SLAndroidSimpleBufferQueueItf androidbufferque = NULL;
static SLVolumeItf volume = NULL;
static SLEffectSendItf effectsend = NULL;
static SLuint32 playerstate = OPENSLES_PLAYERSTATE_UNINITED;
static SLmillisecond duration = 0;
const static SLEnvironmentalReverbSettings envsettings =
        SL_I3DL2_ENVIRONMENT_PRESET_STONEROOM;

static void ReleasePlayer(void);

void BufferQueuePlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    void *buffer = NULL;
    size_t buffer_size = 0;
    int result = getAudioSource(&buffer, &buffer_size);
    if (buffer != NULL && buffer_size != 0 && result == 0) {
        (*bq)->Enqueue(androidbufferque, buffer, (SLuint32) buffer_size);
    } else {
        StopPlay();
    }
}

// create the engine and output mix objects
int createEngine(VideoInfo *videoInfo) {
    SLresult result;
    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    EXITFUN
    // realize the engine, no async needed
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    EXITFUN
    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    EXITFUN
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    EXITFUN
    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    EXITFUN
    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &envrev);
    if (SL_RESULT_SUCCESS == result) {
        result = (*envrev)->SetEnvironmentalReverbProperties(
                envrev, &envsettings);
//        EXITFUN
//        return 0;
    }
    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 1};
    SLAndroidDataFormat_PCM_EX format_android;
    format_android.formatType = SL_DATAFORMAT_PCM;
    format_android.sampleRate = (SLuint32) videoInfo->samplerate * 1000;
    format_android.numChannels = (SLuint32) videoInfo->channels;
    format_android.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_android.containerSize = 16;
    if (videoInfo->channels == 2) {
        format_android.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    } else {
        format_android.channelMask = SL_SPEAKER_FRONT_CENTER;
    }
    format_android.endianness = SL_BYTEORDER_LITTLEENDIAN;
    format_android.representation = SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT;
    duration = videoInfo->duration;
    SLDataSource audioSrc = {&loc_bufq, &format_android};
    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    const SLInterfaceID ids[3] = {
            SL_IID_EFFECTSEND,
            SL_IID_VOLUME,
            SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[3] = {
            SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE
    };
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerobj, &audioSrc, &audioSnk, 3,
                                                ids, req);
    EXITFUN
    result = (*playerobj)->Realize(playerobj, SL_BOOLEAN_FALSE);
    EXITFUN
    result = (*playerobj)->GetInterface(playerobj, SL_IID_PLAY, &player);
    EXITFUN
    result = (*playerobj)->GetInterface(playerobj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                        &androidbufferque);
    EXITFUN
    result = (*androidbufferque)->RegisterCallback(androidbufferque, BufferQueuePlayerCallback,
                                                   NULL);
    EXITFUN
    result = (*playerobj)->GetInterface(playerobj, SL_IID_VOLUME, &volume);
    EXITFUN;
    result = (*playerobj)->GetInterface(playerobj, SL_IID_EFFECTSEND, &effectsend);
    EXITFUN;
    return 0;
}

void ReleasePlayer(void) {
    if (playerobj != NULL) {
        (*playerobj)->Destroy(playerobj);
        playerobj = NULL;
        player = NULL;
        volume = NULL;
        androidbufferque = NULL;
        effectsend = NULL;
    }
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        envrev = NULL;
    }
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    playerstate = OPENSLES_PLAYERSTATE_UNINITED;
    duration = 0;
}

int CreatePlayerInstance(MediaInfo *mediaInfo) {
    if (!mediaInfo) {
        return -1;
    }
    const char *filePath = mediaInfo->uri;
    char errMsg[1024];
    VideoInfo *videoInfo = malloc(sizeof(VideoInfo));
    int result = CreateDecoder(filePath, videoInfo);
    if (result != 0) {
        sprintf(errMsg, "Error in create decoder, file path: %s\n", filePath);
        goto RELEASE;
    }
    if (videoInfo == NULL) {
        sprintf(errMsg, "Error in allocate video info.\n");
        goto RELEASE;
    }
    result = createEngine(videoInfo);
    if (result != 0) {
        sprintf(errMsg, "Error in init audio engine, file path: %s\n", filePath);
        goto RELEASE;
    }
    free(videoInfo);
    playerstate = OPENSLES_PLAYERSTATE_PREPARED;
    TimerParameters *parameters = malloc(sizeof(TimerParameters));
    parameters->duration = duration;
    parameters->getPlayPosition = getPlayPosition;
    parameters->getPlayerState = getPlayState;
    free(mediaInfo);
    createTimer(parameters);
    return 0;
    RELEASE:
    if (strlen(errMsg) > 0) {
        LOGE("%s", errMsg);
        free(errMsg);
    }
//    ReleaseAll();
    return -1;
}

void ReleaseAll(void) {
    PLAYEXIT
    ReleaseResources();
    ReleasePlayer();
}

void StartPlay(void) {
    PLAYEXIT
    if (getPlayState() == SL_PLAYSTATE_PLAYING) {
        return;
    }
    (*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING);
    BufferQueuePlayerCallback(androidbufferque, NULL);
    startTimer();
}

void StopPlay(void) {
    PLAYEXIT;
    (*player)->SetPlayState(player, SL_PLAYSTATE_STOPPED);
    ReleaseAll();
}

void PausePlay(void) {
    PLAYEXIT;
    (*player)->SetPlayState(player, SL_PLAYSTATE_PAUSED);
}

void ResumePlay(void) {
    PLAYEXIT;
    (*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING);
}

SLmillisecond getPlayPosition(void) {
    SLmillisecond time = 0;
    (*player)->GetPosition(player, &time);
    return time;
}

SLuint32 getPlayState(void) {
    SLuint32 state = 0;
    (*player)->GetPlayState(player, &state);
    return state;
}
