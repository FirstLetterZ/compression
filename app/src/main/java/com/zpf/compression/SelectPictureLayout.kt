package com.zpf.compression

import android.os.Build
import android.os.Bundle
import android.view.View
import com.zpf.compress.CompressManager
import com.zpf.support.base.ViewProcessor
import com.zpf.support.util.LogUtil
import com.zpf.tool.FileUtil
import com.zpf.tool.PublicUtil
import com.zpf.tool.toast.ToastUtil
import java.io.File

class SelectPictureLayout : ViewProcessor() {

    override fun getLayoutId(): Int {
        return R.layout.layout_select_picture
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        LogUtil.setLogOut(true)
        LogUtil.setTAG("ZPF")
        bind<View>(R.id.tv_test01)
        bind<View>(R.id.tv_test02)
        Thread {
            logFileType("pic_webp_example.webp")
            logFileType("pic_png_example.png")
            logFileType("pic_jpg_example.jpg")
            copySoFile()
        }.start()
        Build.SUPPORTED_ABIS
        LogUtil.e("CPU_ABI======>" + Build.CPU_ABI)
        LogUtil.e("screen height=" + PublicUtil.getDisplayMetrics().heightPixels)
        LogUtil.e("screen width =" + PublicUtil.getDisplayMetrics().widthPixels)
    }

    private fun logFileType(name: String) {
        val target = File(FileUtil.getAppCachePath(), name)
        if (!target.exists()) {
            FileUtil.writeToFile(
                target.absolutePath,
                context.assets.open(name)
            )
        }
    }

    private fun copySoFile() {
        val soDirPath = CompressManager.getLibsDirectoryPath(context)
        val target = File(soDirPath, "libjpeg.so")
        if (!target.exists()) {
            FileUtil.writeToFile(
                target.absolutePath,
                context.assets.open("libjpeg.so")
            )
        }
    }

    override fun onClick(view: View?) {
        when (view?.id) {
            R.id.tv_test01 -> {
                val pngFile = File(FileUtil.getAppCachePath(), "pic_png_example.png")
                if (!pngFile.exists()) {
                    ToastUtil.toast("文件不存在")
                    return
                }
                testPngCompress(pngFile.absolutePath)
            }
            R.id.tv_test02 -> {
                val jpgFile = File(FileUtil.getAppCachePath(), "pic_jpg_example.jpg")
                if (!jpgFile.exists()) {
                    ToastUtil.toast("文件不存在")
                    return
                }
                testJpgCompress(jpgFile.absolutePath)
            }
        }
    }

    private fun testJpgCompress(sourcePath: String) {
//        if (!CompressManager.hasLoadNative()) {
//            if (!CompressManager.loadDownloadSoFile(context, "libjpeg.so")) {
//                CompressManager.loadLocalSoFile("jpeg")
//            }
//        }
        val newFile = File(
            FileUtil.getAppCachePath(),
            "compress_" + System.currentTimeMillis() + ".jpg"
        )
        LogUtil.e("start testJpgCompress======>File size= " + File(sourcePath).length())
        val startTime = System.currentTimeMillis()
        try {
            val result =
                CompressManager.compress(
                    sourcePath,
                    newFile.absolutePath,
                    PublicUtil.getDisplayMetrics().widthPixels,
                    PublicUtil.getDisplayMetrics().heightPixels,
                    80
                )
            LogUtil.e("result=$result")
            LogUtil.e("after compress file size= " + newFile.length())
        } catch (e: Throwable) {
            LogUtil.e("Error==>" + e.message)
        }
        LogUtil.e("耗时==>" + (System.currentTimeMillis() - startTime))
    }


    private fun testPngCompress(sourcePath: String) {
        val newFile = File(
            FileUtil.getAppCachePath(),
            "compress_" + System.currentTimeMillis() + ".png"
        )
        LogUtil.e("start testPngCompress======>File size= " + File(sourcePath).length())
        val startTime = System.currentTimeMillis()
        try {
            val result =
                CompressManager.compress(
                    sourcePath,
                    newFile.absolutePath,
                    PublicUtil.getDisplayMetrics().widthPixels,
                    PublicUtil.getDisplayMetrics().heightPixels,
                    80
                )
            LogUtil.e("result=$result")
            LogUtil.e("after compress file size= " + newFile.length())
        } catch (e: Throwable) {
            LogUtil.e("Error==>" + e.message)
        }
        LogUtil.e("耗时==>" + (System.currentTimeMillis() - startTime))
    }
}