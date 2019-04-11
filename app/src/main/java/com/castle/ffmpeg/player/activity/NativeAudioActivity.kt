package com.castle.ffmpeg.player.activity

import android.content.res.AssetManager
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.castle.ffmpeg.player.R
import kotlinx.android.synthetic.main.activity_native_audio.*

class NativeAudioActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_native_audio)
        createEngine()
        btn_play_audio.setOnClickListener { onPlayAudioClick() }
    }

    var isPlayingAsset = false

    private val mManager by lazy { assets }

    private fun onPlayAudioClick() {
        if (createAssetAudioPlayer(mManager, "background.mp3")) {
            isPlayingAsset = !isPlayingAsset;
            setPlayingAssetAudioPlayer(true)
        }
    }

    private external fun createAssetAudioPlayer(assetManager: AssetManager, filePath: String): Boolean

    private external fun setPlayingAssetAudioPlayer(play: Boolean)
    private external fun createEngine()
    private external fun shutdown()

    override fun onDestroy() {
        super.onDestroy()
        shutdown()
    }

    /** Called when the activity is about to be destroyed. */
    override fun onPause() {
        isPlayingAsset = false
        setPlayingAssetAudioPlayer(false)
        super.onPause()
    }
}
