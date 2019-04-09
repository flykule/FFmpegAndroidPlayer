#include <jni.h>
#include <string>

extern "C" {
#include "libavcodec/avcodec.h"
}

extern "C"
JNIEXPORT jstring

JNICALL
Java_com_castle_ffmpeg_player_activity_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
}

