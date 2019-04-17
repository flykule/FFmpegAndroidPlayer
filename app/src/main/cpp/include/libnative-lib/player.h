//
// Created by castle on 4/17/19.
//

#ifndef FFMPEGANDROIDPLAYER_PLAYER_H
#define FFMPEGANDROIDPLAYER_PLAYER_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <libnative-lib/mediatimer.h>
#include <libnative-lib/util.h>
#include <libnative-lib/mediainfo.h>

void StartPlay(void);

void StopPlay(void);

void PausePlay(void);

void ResumePlay(void);

void ReleaseAll(void);

int CreatePlayerInstance(MediaInfo *mediaInfo);

SLmillisecond getPlayPosition(void);

SLuint32 getPlayState(void);

#endif //FFMPEGANDROIDPLAYER_PLAYER_H
