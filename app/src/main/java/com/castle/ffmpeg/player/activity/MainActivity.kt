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

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        Timber.tag(this::class.java.simpleName)
        printStartLog()
        saveAssetsVideoToFilesDir()
    }

    lateinit var mOriginVideoPath: String;

    private fun saveAssetsVideoToFilesDir() {
        val stream = assets.open("short.mp4")
        if (stream.available() > 0) {
            val path = filesDir.path + "/short.mp4"
            mOriginVideoPath = path
            val fileAsync = stream.toFileAsync(path)
            runBlocking {
                fileAsync.await()
                Timber.e("Local video ready, path: %s", path)
                printVideoInfo(path)
                delay(1000)
                extractImage()
            }
        }
    }

    /**
     * Extract 1 frame from ffmpeg first
     */
    private fun extractImage() {
//        ffmpeg -i ./short.mp4 -r 1 -frames:v 1 images/test.jpeg
        val imagePath = cacheDir.path + "/test.jpeg"
        FFmpeg.getInstance(this).exec("-i", mOriginVideoPath, "-r", "1", "-frames:v", "1", imagePath, handler = object : ExecuteBinaryResponseHandler() {
            override fun onFailure(message: String?) {
                Timber.e(message ?: "Some error happened!")
            }

            override fun onFinish() {
                Timber.e("Extract finish")
            }

            override fun onSuccess(message: String?) {
                Timber.e(message)
                iv_image.setImageBitmap(BitmapFactory.decodeFile(imagePath))
            }
        })
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
