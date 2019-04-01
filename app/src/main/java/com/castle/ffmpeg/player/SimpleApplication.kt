package com.castle.ffmpeg.player

import android.app.Application
import timber.log.Timber

class SimpleApplication : Application() {

    companion object {
        lateinit var mInstance: SimpleApplication
    }

    override fun onCreate() {
        super.onCreate()
        mInstance = this
        Timber.plant(Timber.DebugTree())
    }
}
