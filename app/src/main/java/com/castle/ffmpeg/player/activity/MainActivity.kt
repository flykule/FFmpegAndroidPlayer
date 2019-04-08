package com.castle.ffmpeg.player.activity

import android.graphics.BitmapFactory
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.castle.ffmpeg.player.R
import com.castle.ffmpeg.player.extensions.exec
import com.castle.ffmpeg.player.extensions.toFileAsync
import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import nl.bravobit.ffmpeg.ExecuteBinaryResponseHandler
import nl.bravobit.ffmpeg.FFmpeg
import nl.bravobit.ffmpeg.FFprobe
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
            } catch (e: UnsatisfiedLinkError) {
                e.printStackTrace()
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        Timber.tag(this::class.java.simpleName)
        printStartLog()
    }

    private fun printStartLog() {

    }
}
