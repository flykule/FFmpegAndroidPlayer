package com.castle.ffmpeg.player.view

import android.content.Context
import android.graphics.PixelFormat
import android.util.AttributeSet
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.async
import timber.log.Timber

class FFSurfaceView : SurfaceView, SurfaceHolder.Callback {

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {

    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {

    }

    override fun surfaceCreated(holder: SurfaceHolder?) {
        Timber.d("Surface created, start render")
        if (!mVideoPath.isEmpty()) {
            val deferred = playerAsync(mVideoPath)
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
        holder.addCallback(this);
    }

    private var mVideoPath: String = ""

    fun playerAsync(input: String) = GlobalScope.async {
        mVideoPath = input
        if (this@FFSurfaceView.holder.surface != null) {
            render(input, this@FFSurfaceView.holder.surface)
            return@async
        }
        Timber.d("Surface not created, wait for create")
    }

    private external fun render(input: String, surface: Surface)

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
