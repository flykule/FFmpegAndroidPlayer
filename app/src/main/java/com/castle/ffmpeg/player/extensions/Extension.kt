package com.castle.ffmpeg.player.extensions

import android.app.Activity
import android.content.Context
import android.content.ContextWrapper
import android.content.Intent
import android.content.res.Resources
import android.graphics.drawable.Drawable
import android.os.Build
import android.os.Parcelable
import android.text.TextUtils
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import android.widget.Toast
import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.async
import java.io.File
import java.io.InputStream
import kotlin.jvm.internal.CallableReference
import kotlin.reflect.KClass
import kotlin.reflect.KDeclarationContainer
import kotlin.reflect.KProperty

/**
 * Created by castle on 17-1-9.
 * Kotlin的通用扩展函数以及属性，全部放在一起
 */
//扩展属性，获取view的上下文
val View.ctx: Context
    get() = context
//扩展属性，快速判断可见性
val View.visible: Boolean
    get() = visibility == View.VISIBLE
// /扩展属性，快速判断可见性
val View.gone: Boolean
    get() = visibility == View.GONE

// /扩展属性，快速判断可见性
val View.invisible: Boolean
    get() = visibility == View.INVISIBLE
//扩展属性，获取资源
val Activity.res: Resources
    get() = resources

//扩展方法，获取颜色
fun Context.color(resId: Int): Int = ContextCompat.getColor(this, resId)

//扩展方法，获取Drawable
fun Context.drawable(resId: Int): Drawable = ContextCompat.getDrawable(this, resId)
        ?: throw NullPointerException()

//扩展方法，获取string
fun Context.string(resId: Int) = this.getString(resId) ?: ""

//扩展方法，获取尺寸
fun Context.dimension(resId: Int) = resources.getDimension(resId)

//扩展方法，获取颜色
fun View.color(resId: Int): Int = ContextCompat.getColor(context, resId)

//扩展方法，获取Drawable,空的话直接丢出空指针
fun View.drawable(resId: Int): Drawable = ContextCompat.getDrawable(context, resId)
        ?: throw NullPointerException()

//扩展方法，获取string
fun View.string(resId: Int) = context.string(resId)

//扩展方法，获取尺寸
fun View.dimens(resId: Int) = resources.getDimension(resId)

fun View.switchEnable() {
    isEnabled = !isEnabled
}

//获取指定ViewGroup的特定子view
operator fun ViewGroup.get(position: Int): View = getChildAt(position)

//只支持AndroidM以上版本
fun supportM(f: () -> Unit) {
    if (Build.VERSION.SDK_INT > Build.VERSION_CODES.M) {
        return f()
    }
}

//只支持AndroidLOLLIPOP以上版本
inline fun supportLOLLIPOP(f: () -> Unit) {
    if (Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP) {
        return f()
    }
}

//toast显示错误信息
fun Activity.toast(@StringRes msg: Int, length: Int = Toast.LENGTH_SHORT) {
    toast(string(msg), length)
}

//toast显示错误信息
fun Context.toast(msg: String, length: Int = Toast.LENGTH_SHORT) {
    toast(msg, length)
//    ToastUtil.showToastCenter(this, msg, length)
}

//toast显示错误信息
fun Context.toast(@StringRes msg: Int, length: Int = Toast.LENGTH_SHORT) {
    toast(string(msg), length)
}

//注册Rxbus
fun Activity.regRxbus() {
//    RxBus.get().register(this)
}

//注销Rxbus
fun Activity.unregRxbus() {
//    RxBus.get().unregister(this)
}

//快速判断字符串是否为空
fun String.isEmpty(): Boolean {
    return TextUtils.isEmpty(this)
}


//执行只在5.0以上运行的代码
inline fun Context.plusLolipop(f: () -> Unit): Boolean {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
        f()
        return true
    } else {
        return false
    }
}

/**
 * 设置滑行离开动画
 */
fun View.slideExit() {
    if (translationY == 0f) animate().translationY(-height.toFloat())
}

/**
 * 设置滑行进入动画
 */
fun View.slideEnter() {
    if (translationY < 0f) animate().translationY(0f)
}

val KProperty<*>.ownerCanonicalName: String? get() = owner?.canonicalName
val KProperty<*>.owner: KDeclarationContainer? get() = if (this is CallableReference) owner else null
val KDeclarationContainer.canonicalName: String? get() = if (this is KClass<*>) this.java.canonicalName else null

/**
 * intent String委托代理类，用法参见LifeCircleRegionActivity
 */
class IntentExtraString(private val name: String? = null) {

    private val KProperty<*>.extraName get() = this@IntentExtraString.name ?: fallbackName
    //这个it指的就是ownerCanonicalName也就是这个属性的类的全名,这个$name指的则是变量的名字
    private val KProperty<*>.fallbackName get() = ownerCanonicalName?.let { "$it::$name" } ?: name

    operator fun getValue(intent: Intent, property: KProperty<*>): String? =
            intent.getStringExtra(property.extraName)

    operator fun setValue(intent: Intent, property: KProperty<*>, value: String?) {
        intent.putExtra(property.extraName, value)
    }
}

/**
 * intent String委托代理类，用法参见LifeCircleRegionActivity
 */
class IntentExtraBoolean(private val name: String? = null) {

    private val KProperty<*>.extraName get() = this@IntentExtraBoolean.name ?: fallbackName
    //这个it指的就是ownerCanonicalName也就是这个属性的类的全名,这个$name指的则是变量的名字
    private val KProperty<*>.fallbackName get() = ownerCanonicalName?.let { "$it::$name" } ?: name

    operator fun getValue(intent: Intent, property: KProperty<*>): Boolean? =
            intent.getBooleanExtra(property.extraName, false)

    operator fun setValue(intent: Intent, property: KProperty<*>, value: Boolean?) {
        intent.putExtra(property.extraName, value)
    }
}

/**
 * intent long委托代理类，用法参见LifeCircleRegionActivity
 */
class IntentExtraLong(private val name: String? = null) {

    private val KProperty<*>.extraName get() = this@IntentExtraLong.name ?: fallbackName
    private val KProperty<*>.fallbackName get() = ownerCanonicalName?.let { "$it::$name" } ?: name

    operator fun getValue(intent: Intent, property: KProperty<*>): Long? =
            intent.getLongExtra(property.extraName, 0)

    operator fun setValue(intent: Intent, property: KProperty<*>, value: Long?) {
        intent.putExtra(property.extraName, value)
    }
}

/**
 * intent int委托代理类，用法参见LifeCircleRegionActivity
 */
class IntentExtraInt(private val name: String? = null) {

    private val KProperty<*>.extraName get() = this@IntentExtraInt.name ?: fallbackName
    private val KProperty<*>.fallbackName get() = ownerCanonicalName?.let { "$it::$name" } ?: name

    operator fun getValue(intent: Intent, property: KProperty<*>): Int? =
            intent.getIntExtra(property.extraName, 0)

    operator fun setValue(intent: Intent, property: KProperty<*>, value: Int?) {
        intent.putExtra(property.extraName, value)
    }
}

/**
 * intent Serizable委托代理类，用法参见LifeCircleRegionActivity，并强转为范型
 */
class IntentExtraParcelable<out T : Parcelable, in R : T>(private val name: String? = null) {

    private val KProperty<*>.extraName get() = this@IntentExtraParcelable.name ?: fallbackName
    private val KProperty<*>.fallbackName get() = ownerCanonicalName?.let { "$it::$name" } ?: name

    @SuppressWarnings("unchecked")
    operator fun getValue(intent: Intent, property: KProperty<*>): T? {
        return intent.getParcelableExtra(property.extraName) as? T
    }

    operator fun setValue(intent: Intent, property: KProperty<*>, value: R?) {
        intent.putExtra(property.extraName, value)
    }
}

/**
 * 从context中读取出Activity
 */
fun Context?.getActivity(): Activity? {
    if (this == null) {
        return null
    } else if (this is ContextWrapper) {
        return this as? Activity ?: this.baseContext.getActivity()
    }
    return null
}

/**
 * intent Serizable委托代理类，用法参见LifeCircleRegionActivity，并强转为范型
 */
//class IntentExtraSerizable<out T : java.io.Serializable, in R : T>(private val name: String? = null) {
//
//    private val KProperty<*>.extraName get() = this@IntentExtraSerizable.name ?: fallbackName
//    private val KProperty<*>.fallbackName get() = ownerCanonicalName?.let { "$it::$name" } ?: name
//
//    @SuppressWarnings("unchecked")
//    operator fun getValue(intent: Intent, property: KProperty<*>): T? {
//        val extra = intent.getSerializableExtra(property.extraName)
////        if (extra.javaClass == rClass.javaClass){
//        return extra as R
////        }
////        return null
//    }
//
//    operator fun setValue(intent: Intent, property: KProperty<*>, value: R) {
//        intent.putExtra(property.extraName, value)
//    }
//}

/**
 * 设置textview的图片
 */
fun TextView.setDrawable(@DrawableRes leftImg: Int = 0, @DrawableRes topImg: Int = 0,
                         @DrawableRes rightImg: Int = 0, @DrawableRes bottomImg: Int = 0) {
    setCompoundDrawablesWithIntrinsicBounds(leftImg, topImg, rightImg, bottomImg)
}

/**
 * 设置textview的图片
 */
fun TextView.setDrawable(leftImg: Drawable? = null, topImg: Drawable? = null,
                         rightImg: Drawable? = null, bottomImg: Drawable? = null) {
    setCompoundDrawablesWithIntrinsicBounds(leftImg, topImg, rightImg, bottomImg)
}

/**
 * 只有对象是null的时候才进行的方法
 */
inline fun <reified T> T?.onNull(f: () -> Unit) {
    if (this == null) f()
}

/**
 * 在集合中间插入字符
 */
fun List<Any>.addFlag(flag: String): String {
    return fold("") { acc, s ->
        acc + s + flag
    }.substringBeforeLast(flag)
}

//使用这个拓展开启Activity
inline fun <reified T : Activity> Context.startActivity() {
    startActivity(Intent(this, T::class.java))
}

//用于获取字符串
inline fun <reified T : Any> Activity.extra(key: String, default: T? = null) = lazy {
    val value = intent?.extras?.get(key)
    if (value is T) value else default
}

inline fun <reified T : Any> Activity.extraNotNull(key: String, default: T? = null) = lazy {
    val value = intent?.extras?.get(key)
    requireNotNull(if (value is T) value else default) { key }
}

inline fun <reified T : Any> Fragment.extra(key: String, default: T? = null) = lazy {
    val value = arguments?.get(key)
    if (value is T) value else default
}

inline fun <reified T : Any> Fragment.extraNotNull(key: String, default: T? = null) = lazy {
    val value = arguments?.get(key)
    requireNotNull(if (value is T) value else default) { key }
}

fun InputStream.toFileAsync(path: String) = GlobalScope.async {
    val stream = File(path).outputStream()
    try {
        copyTo(stream, 4096)
    } catch (e: Exception) {
        e.printStackTrace()
    } finally {
        stream.close()
        close()
    }
}

//fun FFmpeg.exec(vararg args: String,handler: FFcommandExecuteResponseHandler = TimberResponseHandler) {
//    execute(args, handler)
//}
//
//fun FFprobe.exec(vararg args: String,handler: FFcommandExecuteResponseHandler = TimberResponseHandler) {
//    execute(args, handler)
//}
//
//fun Context.toFFmpegCommand(command: String) = command.split(" ").toTypedArray()


