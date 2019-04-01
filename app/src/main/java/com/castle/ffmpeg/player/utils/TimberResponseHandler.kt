package com.castle.ffmpeg.player.utils

import nl.bravobit.ffmpeg.ExecuteBinaryResponseHandler
import timber.log.Timber

object TimberResponseHandler : ExecuteBinaryResponseHandler() {
    override fun onSuccess(message: String) {
        Timber.e(message)
    }

    override fun onProgress(message: String) {
        Timber.e(message)
    }
}
