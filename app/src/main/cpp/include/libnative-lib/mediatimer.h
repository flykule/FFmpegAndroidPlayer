//
// Created by castle on 4/16/19.
//

#ifndef FFMPEGANDROIDPLAYER_MEDIATIMER_H
#define FFMPEGANDROIDPLAYER_MEDIATIMER_H

#include <SLES/OpenSLES.h>

typedef struct TimerParameters {
    int64_t duration;  //资源总时长
    SLuint32 (*getPlayerState)(void); //当前播放状态
    SLmillisecond (*getPlayPosition)(void); //当前播放进度
} TimerParameters;

int createTimer(TimerParameters *parameters);

int startTimer(void);

#endif //FFMPEGANDROIDPLAYER_MEDIATIMER_H
