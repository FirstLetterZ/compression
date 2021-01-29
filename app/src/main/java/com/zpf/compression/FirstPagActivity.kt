package com.zpf.compression

import com.zpf.frame.IViewProcessor
import com.zpf.support.base.CompatContainerActivity

class FirstPagActivity : CompatContainerActivity() {

    override fun launcherViewProcessorClass(): Class<out IViewProcessor> {
        return SelectPictureLayout::class.java
    }
}