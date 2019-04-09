package com.castle.ffmpeg.player.view

import android.content.Context
import android.graphics.PixelFormat
import android.util.AttributeSet
import android.view.Surface
import android.view.SurfaceView
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.async

class FFSurfaceView : SurfaceView {

    constructor(context: Context?) : super(context) {
        init()
    }

    constructor(context: Context?, attrs: AttributeSet?) : super(context, attrs) {
        init()
    }

    private fun init() {
//        val holder = holder
        holder.setFormat(PixelFormat.RGBA_8888)
    }

    fun playerAsync(input: String) = GlobalScope.async {
        //        绘制功能 不需要交给SurfaveView        VideoView.this.getHolder().getSurface()
        render(input, this@FFSurfaceView.holder.surface)
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
