package com.castle.ffmpeg.player.activity

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.castle.ffmpeg.player.R
import com.castle.ffmpeg.player.extensions.toFileAsync
import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import timber.log.Timber
import java.io.File


class MainActivity : AppCompatActivity() {

    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            try {
                //Order is important!
                System.loadLibrary("avutil")
                System.loadLibrary("swresample")
                System.loadLibrary("avcodec")
                System.loadLibrary("avformat")
                System.loadLibrary("swscale")
                System.loadLibrary("avfilter")
                System.loadLibrary("avdevice")
//                System.loadLibrary("videokit")
                System.loadLibrary("native_lib")
            } catch (e: UnsatisfiedLinkError) {
                e.printStackTrace()
            }
        }
    }

    private external fun stringFromJNI(): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        Timber.tag(this::class.java.simpleName)
        printStartLog()
        tv_center.setOnClickListener { saveAssetsVideoToFilesDir() }
    }


    lateinit var mOriginVideoPath: String;

    private fun saveAssetsVideoToFilesDir() = runBlocking {
        launch {
            val path = filesDir.path + "/short.mp4"
            val file = File(path)
            if (!file.exists()) {
                val stream = assets.open("short.mp4")
                if (stream.available() > 0) {
                    mOriginVideoPath = path
                    val fileAsync = stream.toFileAsync(path)
                    fileAsync.await()
                }
            }
            Timber.e("Local video ready, path: %s", path)
//            ffs_bottom.get
            val async = this@MainActivity.ffs_bottom.playerAsync(path)
            async.await()
            return@launch
        }
    }

    private fun printStartLog() {
//        Timber.d("Hello from Jni: %s", stringFromJNI())
//        tv_center.text = stringFromJNI()
    }
}
