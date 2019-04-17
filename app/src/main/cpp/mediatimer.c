//
// Created by castle on 4/16/19.
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <libnative-lib/mediatimer.h>
#include <unistd.h>
#include <libnative-lib/util.h>

static void *MediaTimer(void *);
static void ReleaseTimer(void);
static pthread_t thread_timer_id = 0;
static pthread_cond_t pcondt = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t pthreadMutex = PTHREAD_MUTEX_INITIALIZER;

int createTimer(TimerParameters *parameters) {
    if (parameters == NULL) {
        return -1;
    }
    if (parameters->duration <= 0) {
        return -1;
    }
    if (parameters->getPlayPosition == NULL) {
        return -1;
    }
    if (parameters->getPlayerState == NULL) {
        return -1;
    }
    pthread_create(&thread_timer_id, NULL, MediaTimer, parameters);
    return 0;
}

int startTimer(void) {
    if (!thread_timer_id) {
        return -1;
    }
    pthread_cond_signal(&pcondt);
    pthread_join(thread_timer_id, NULL);
    ReleaseTimer();
    return 0;
}

void ReleaseTimer() {
    pthread_cond_destroy(&pcondt);
    pthread_mutex_destroy(&pthreadMutex);
    thread_timer_id = 0;
}

void *MediaTimer(void *params) {
    pthread_mutex_lock(&pthreadMutex);
    TimerParameters *timerParameters = (TimerParameters *) params;
    pthread_cond_wait(&pcondt, &pthreadMutex);
    while (timerParameters->getPlayerState() == SL_PLAYSTATE_PLAYING) {
        sleep(1);
        if (timerParameters->getPlayerState() != SL_PLAYSTATE_PLAYING) {
            break;
        }
        LOGD("位置/时长: %d/%d\n", timerParameters->getPlayPosition(), timerParameters->duration);
    }
    free(params);
    pthread_mutex_unlock(&pthreadMutex);
    return NULL;
}
