package com.castle.ffmpeg.player.view

import android.content.Context
import android.graphics.PixelFormat
import android.media.AudioManager
import android.os.Build
import android.util.AttributeSet
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import com.castle.ffmpeg.player.activity.NativeAudioActivity
import com.castle.ffmpeg.player.activity.NativeAudioActivity.Companion.createBufferQueueAudioPlayer
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.async
import timber.log.Timber

class FFSurfaceView : SurfaceView, SurfaceHolder.Callback {

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {

    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
        mSurfaceCreated = false
        shutdown()
    }

    var mSurfaceCreated = false

    override fun surfaceCreated(holder: SurfaceHolder?) {
        mSurfaceCreated = true
        Timber.d("Surface created, start render")
        if (!mVideoPath.isEmpty()) {
            val async = playerAsync(mVideoPath)
        }
    }

    constructor(context: Context?) : super(context) {
        init()
    }

    constructor(context: Context?, attrs: AttributeSet?) : super(context, attrs) {
        init()
    }

    private fun init() {
        Timber.tag(this::class.java.simpleName)
        holder.setFormat(PixelFormat.RGBA_8888)
        holder.addCallback(this)
//        initAudioEngine()
    }

    private var mVideoPath: String = ""

    fun playerAsync(input: String) = GlobalScope.async {
        mVideoPath = input
        Timber.d("Current surface %s isCreating %s", this@FFSurfaceView.holder.surface, this@FFSurfaceView.holder.isCreating)
        if (mSurfaceCreated) {
            if (createPlayer(mVideoPath)) {
                setPlayingState(true)
            }
//            render(input, this@FFSurfaceView.holder.surface)
            mVideoPath = ""
            return@async
        }
        Timber.d("Surface not created, wait for create")
    }

    private external fun render(input: String, surface: Surface)
    private external fun createPlayer(input: String): Boolean
    private external fun setPlayingState(play: Boolean)
    private external fun shutdown()

    /**
     * Use quick audio path and sample rate
     */
    private fun initAudioEngine() {
        NativeAudioActivity.createEngine()
        var sampleRate = 0
        var bufSize = 0
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            val myAudioMgr = context.getSystemService(Context.AUDIO_SERVICE) as AudioManager
            var nativeParam = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE)
            sampleRate = Integer.parseInt(nativeParam)
            nativeParam = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER)
            bufSize = Integer.parseInt(nativeParam)
        }
        createBufferQueueAudioPlayer(sampleRate, bufSize)
    }

    companion object {
        init {
            try {
                System.loadLibrary("avutil")
                System.loadLibrary("swresample")
                System.loadLibrary("avcodec")
                System.loadLibrary("avformat")
                System.loadLibrary("swscale")
                System.loadLibrary("avfilter")
                System.loadLibrary("avdevice")
//                System.loadLibrary("videokit")
                System.loadLibrary("native_lib")
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }
}
