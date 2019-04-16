//
// Created by castle on 4/16/19.
//

#ifndef FFMPEGANDROIDPLAYER_MEDIAINFO_H
#define FFMPEGANDROIDPLAYER_MEDIAINFO_H

#include <libnative-lib/mediatimer.h>
#include <stdlib.h>

typedef struct MediaInfo {
    char *uri;
    TimerParameters *tParams;
} MediaInfo;

#endif //FFMPEGANDROIDPLAYER_MEDIAINFO_H
