package com.zpf.compress;

import android.annotation.SuppressLint;
import android.content.Context;

import java.io.File;
import java.io.InputStream;

public class CompressManager {
    private static final String libsPath = "libs";
    private static boolean hasLoadNative = false;

    private static class JpgLib {
        static com.zpf.compress.jpglib.CompressUtil instance = new com.zpf.compress.jpglib.CompressUtil();
    }

    //用于兼容Android Q，以及仅能通过uri读取文件流的情况
    public static int compress(InputStream sourceStream, String targetFilePath,
                               int outWidth, int outHeight, int quality) {
        if (sourceStream == null || targetFilePath == null || quality < 0 || quality > 100) {
            return CompressErrorCode.ERROR_CHECK_OPTION;
        }
        int result = AndroidCompressUtil.compress(sourceStream, targetFilePath, outWidth, outHeight, quality);
        if (result < 0) {
            File outFile = new File(targetFilePath);
            if (outFile.exists()) {
                outFile.delete();
            }
        }
        return result;
    }

    public static int compress(String sourceFilePath, String targetFilePath, int outWidth, int outHeight,
                               int quality) {
        return compress(sourceFilePath, targetFilePath, outWidth, outHeight, quality, null);
    }

    //remark暂时未使用
    public static int compress(String sourceFilePath, String targetFilePath, int outWidth, int outHeight,
                               int quality, String remark) {
        if (sourceFilePath == null || targetFilePath == null || quality < 0 || quality > 100) {
            return CompressErrorCode.ERROR_CHECK_OPTION;
        }
        int result;
        if (FileType.PNG == FileTypeUtil.readFileType(sourceFilePath)) {
            result = AndroidCompressUtil.compress(sourceFilePath, targetFilePath, outWidth, outHeight, quality);
        } else {
            if (hasLoadNative) {
                try {
                    result = JpgLib.instance.nativeCompress(sourceFilePath, targetFilePath,
                            outWidth, outHeight, quality, remark);
                } catch (Exception e) {
                    hasLoadNative = false;
                    result = AndroidCompressUtil.compress(sourceFilePath, targetFilePath, outWidth, outHeight, quality);
                }
            } else {
                result = AndroidCompressUtil.compress(sourceFilePath, targetFilePath, outWidth, outHeight, quality);
            }
        }
        if (result < 0) {
            File outFile = new File(targetFilePath);
            if (outFile.exists()) {
                outFile.delete();
            }
        }
        return result;
    }

    public static boolean hasLoadNative() {
        return hasLoadNative;
    }

    public static String getLibsDirectoryPath(Context context) {
        return context.getDir(libsPath, Context.MODE_PRIVATE).getAbsolutePath();
    }

    @SuppressLint("UnsafeDynamicallyLoadedCode")
    public static boolean loadDownloadSoFile(Context context, String soFileName) {
        File file = null;
        try {
            file = new File(getLibsDirectoryPath(context), soFileName);
        } catch (Exception e) {
            //
        }
        if (file != null && file.exists()) {
            try {
                System.load(file.getAbsolutePath());
                hasLoadNative = true;
                return true;
            } catch (Exception e) {
                //
            }
        }
        return false;
    }

    public static boolean loadLocalSoFile(String localLibName) {
        if (localLibName != null && localLibName.length() > 0) {
            try {
                System.loadLibrary(localLibName);
                hasLoadNative = true;
                return true;
            } catch (Exception e) {
                //
            }
        }
        return false;
    }

}