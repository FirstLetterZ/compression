package com.zpf.compress;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.ExifInterface;

import java.io.ByteArrayOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;

class AndroidCompressUtil {

   public static int compress(String sourceFilePath, String targetFilePath, int outWidth, int outHeight, int quality) {
        if (sourceFilePath == null || targetFilePath == null || quality < 0 || quality > 100) {
            return CompressErrorCode.ERROR_CHECK_OPTION;
        }
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        options.inSampleSize = 1;
        BitmapFactory.decodeFile(sourceFilePath, options);
        options.inJustDecodeBounds = false;
        int originalWidth = options.outWidth;
        int originalHeight = options.outHeight;
        if (outHeight < 0) {
            outHeight = originalHeight;
        }
        if (outWidth < 0) {
            outWidth = originalWidth;
        }
        if (outWidth <= 0 || outHeight <= 0) {
            return CompressErrorCode.ERROR_CHECK_OPTION;
        }
        options.inSampleSize = computeSize(outWidth, outHeight);
        Bitmap.CompressFormat format;
        if (FileType.PNG == FileTypeUtil.readFileType(sourceFilePath)) {
            format = Bitmap.CompressFormat.PNG;
        } else {
            format = Bitmap.CompressFormat.JPEG;
        }

        Bitmap sourceBitmap = BitmapFactory.decodeFile(sourceFilePath, options);
        if (sourceBitmap == null) {
            return CompressErrorCode.ERROR_READ_FILE;
        }
        sourceBitmap = rotatingImage(sourceBitmap, readPictureDegree(sourceFilePath));
        try (ByteArrayOutputStream stream = new ByteArrayOutputStream(); FileOutputStream fos = new FileOutputStream(targetFilePath)) {
            sourceBitmap.compress(format, quality, stream);
            fos.write(stream.toByteArray());
            fos.flush();
        } catch (IOException e) {
            return CompressErrorCode.ERROR_WHITE_FILE;
        } finally {
            sourceBitmap.recycle();
        }
        return CompressErrorCode.SUCCESS_ANDROID;
    }

    /**
     * 读取照片旋转角度
     *
     * @param path 照片路径
     * @return 角度
     */
    public static int readPictureDegree(String path) {
        int degree = 0;
        try {
            ExifInterface exifInterface = new ExifInterface(path);
            int orientation = exifInterface.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_NORMAL);
            switch (orientation) {
                case ExifInterface.ORIENTATION_ROTATE_90:
                    degree = 90;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_180:
                    degree = 180;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_270:
                    degree = 270;
                    break;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return degree;
    }

    private static int computeSize(int width, int height) {
        width = width % 2 == 1 ? width + 1 : width;
        height = height % 2 == 1 ? height + 1 : height;
        int longSide = Math.max(width, height);
        int shortSide = Math.min(width, height);
        float scale = ((float) shortSide / longSide);
        if (scale <= 1 && scale > 0.5625) {
            if (longSide < 1664) {
                return 1;
            } else if (longSide < 4990) {
                return 2;
            } else if (longSide > 4990 && longSide < 10240) {
                return 4;
            } else {
                return longSide / 1280;
            }
        } else if (scale <= 0.5625 && scale > 0.5) {
            return longSide / 1280 == 0 ? 1 : longSide / 1280;
        } else {
            return (int) Math.ceil(longSide / (1280.0 / scale));
        }
    }

    private static Bitmap rotatingImage(Bitmap bitmap, int angle) {
        Matrix matrix = new Matrix();
        matrix.postRotate(angle);
        return Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
    }

}