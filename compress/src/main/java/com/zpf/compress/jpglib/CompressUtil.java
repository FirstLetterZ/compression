package com.zpf.compress.jpglib;

public class CompressUtil {

    /**
     * @param sourceFilePath 源文件全路径
     * @param targetFilePath 生成文件全路径
     * @param outWidth       输出图片宽度，<=0 则使用源图片宽度
     * @param outHeight      输出图片高度，<=0 则使用源图片高度
     * @param quality        输出图片质量（1~100）
     * @param remark         备注，暂时未使用
     * @return 1--成功；-11--读取图片失败，-21--写入图片参数异常，-22--创建写入文件异常，-29--写入过程异常
     */
    public native int nativeCompress(
            String sourceFilePath,
            String targetFilePath,
            int outWidth,
            int outHeight,
            int quality,
            String remark
    );
}
