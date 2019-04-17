//
// Created by castle on 4/16/19.
//

#ifndef FFMPEGANDROIDPLAYER_DECODER_H
#define FFMPEGANDROIDPLAYER_DECODER_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <SLES/OpenSLES.h>

typedef struct VideoInfo {
    int channels;
    int samplerate;
    int64_t duration;
} VideoInfo;

int CreateDecoder(const char *filepath, VideoInfo *infos);

int getAudioSource(void **buffer, size_t *buffersize);

void ReleaseResources(void);

#endif //FFMPEGANDROIDPLAYER_DECODER_H
