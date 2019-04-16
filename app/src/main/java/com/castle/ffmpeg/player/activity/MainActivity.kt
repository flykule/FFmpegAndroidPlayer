package com.castle.ffmpeg.player.activity

import android.content.Context
import android.media.AudioManager
import android.os.Build
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.castle.ffmpeg.player.R
import com.castle.ffmpeg.player.activity.NativeAudioActivity.Companion.createBufferQueueAudioPlayer
import com.castle.ffmpeg.player.extensions.startActivity
import com.castle.ffmpeg.player.extensions.toFileAsync
import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import timber.log.Timber
import java.io.File


class MainActivity : AppCompatActivity() {

    private external fun stringFromJNI(): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        Timber.tag(this::class.java.simpleName)
        printStartLog()
        initAudioEngine()
        btn_play.setOnClickListener { saveAssetsVideoToFilesDir() }
        btn_go_audio.setOnClickListener { startActivity<NativeAudioActivity>() }
    }

    private fun initAudioEngine() {
        NativeAudioActivity.createEngine()
        var sampleRate = 0
        var bufSize = 0
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            val myAudioMgr = getSystemService(Context.AUDIO_SERVICE) as AudioManager
            var nativeParam = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE)
            sampleRate = Integer.parseInt(nativeParam)
            nativeParam = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER)
            bufSize = Integer.parseInt(nativeParam)
        }
        createBufferQueueAudioPlayer(sampleRate, bufSize)
    }


    lateinit var mOriginVideoPath: String

    private fun saveAssetsVideoToFilesDir() = runBlocking {
        launch {
            val path = filesDir.path + "/test_cut2.mp4"
            val file = File(path)
            if (!file.exists()) {
                val stream = assets.open("test_cut2.mp4")
                if (stream.available() > 0) {
                    mOriginVideoPath = path
                    val fileAsync = stream.toFileAsync(path)
                    fileAsync.await()
                }
            }
            Timber.e("Local video ready, path: %s", path)

            val async = this@MainActivity.ffs_bottom.playerAsync(path)
            async.await()
            return@launch
        }
    }

    private fun printStartLog() {
        Timber.d("FFmpeg --version %s", stringFromJNI())
    }
}
