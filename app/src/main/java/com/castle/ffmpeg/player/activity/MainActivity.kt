package com.castle.ffmpeg.player.activity

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.castle.ffmpeg.player.R
import com.castle.ffmpeg.player.extensions.exec
import com.castle.ffmpeg.player.extensions.toFileAsync
import kotlinx.coroutines.runBlocking
import nl.bravobit.ffmpeg.FFmpeg
import nl.bravobit.ffmpeg.FFprobe
import timber.log.Timber

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        Timber.tag(this::class.java.simpleName)
        printStartLog()
        saveAssetsVideoToFilesDir()
    }

    private fun saveAssetsVideoToFilesDir() {
        val stream = assets.open("short.mp4")
        if (stream.available() > 0) {
            val path = filesDir.path + "/short.mp4"
            val fileAsync = stream.toFileAsync(path)
            runBlocking {
                fileAsync.await()
                Timber.e("Local video ready, path: %s", path)
                printVideoInfo(path)
            }
        }
    }

    private fun printVideoInfo(path: String) {
        FFprobe.getInstance(this).exec(path)
    }

    private fun printStartLog() {
        if (FFmpeg.getInstance(this).isSupported) {
            versionFFmpeg()
        }
        if (FFprobe.getInstance(this).isSupported) {
            versionFFprobe()
        }
    }

    private fun versionFFmpeg() {
        FFmpeg.getInstance(this).exec("-version")
    }

    private fun versionFFprobe() {
        FFprobe.getInstance(this).exec("-version")
    }
}
