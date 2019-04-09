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
        mSurfaceCreated = false
    }

    var mSurfaceCreated = false

    override fun surfaceCreated(holder: SurfaceHolder?) {
        mSurfaceCreated = true
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
        Timber.d("Current surface %s isCreating %s", this@FFSurfaceView.holder.surface, this@FFSurfaceView.holder.isCreating)
//        if (this@FFSurfaceView.holder.surface != null && !this@FFSurfaceView.holder.surface.toString().contains("null")) {
        if (mSurfaceCreated) {
//            withContext(Dispatchers.Main) {
            render(input, this@FFSurfaceView.holder.surface)
//            }
            mVideoPath = ""
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
