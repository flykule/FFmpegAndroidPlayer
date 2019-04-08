package com.castle.ffmpeg.player.activity

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.castle.ffmpeg.player.R
import timber.log.Timber


class MainActivity : AppCompatActivity() {

    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            try {
                System.loadLibrary("avutil")
                System.loadLibrary("swresample")
                System.loadLibrary("avcodec")
                System.loadLibrary("avformat")
                System.loadLibrary("swscale")
                System.loadLibrary("avfilter")
                System.loadLibrary("avdevice")
                System.loadLibrary("videokit")
                System.loadLibrary("native_lib")
            } catch (e: UnsatisfiedLinkError) {
                e.printStackTrace()
            }
        }
    }

    external fun stringFromJNI(): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        Timber.tag(this::class.java.simpleName)
        printStartLog()
    }

    private fun printStartLog() {
        Timber.d("Hello from Jni: %s", stringFromJNI())
    }
}
