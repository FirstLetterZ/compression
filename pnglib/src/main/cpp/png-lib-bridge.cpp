#include <jni.h>
#include <csetjmp>
#include <cstdio>
#include <malloc.h>
#include <cstring>
#include <android/log.h>
#include "png/png.h"

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "ZPF", __VA_ARGS__))


struct img_info {
    int width;
    int height;
    unsigned char *bytes;
    int bit_depth;
    int color_type;
};


int writePngImageFile(const char *png_file_name, unsigned char *pixels, int width, int height,
                      int bit_depth, int color_type) {
    png_structp png_ptr;
    png_infop info_ptr;
    FILE *png_file = fopen(png_file_name, "wb");
    if (!png_file) {
        return -1;
    }
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == nullptr) {
        LOGE("ERROR:png_create_write_struct/n");
        fclose(png_file);
        return 0;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        LOGE("ERROR:png_create_info_struct/n");
        png_destroy_write_struct(&png_ptr, NULL);
        return 0;
    }
    LOGE("before write======>width=%d;;height=%d;;bit_depth=%d;;color_type=%d", width, height, bit_depth, color_type);
    png_init_io(png_ptr, png_file);

    png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_colorp palette = (png_colorp) png_malloc(png_ptr,
                                                 PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
    if (!palette) {
        fclose(png_file);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }
    png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);
    png_write_info(png_ptr, info_ptr);
    png_set_packing(png_ptr);
    //这里就是图像数据了
    png_bytepp rows = (png_bytepp) png_malloc(png_ptr, height * sizeof(png_bytep));
    for (int i = 0; i < height; ++i) {
        rows[i] = (png_bytep) (pixels + (i) * width * 4);
    }
    png_write_image(png_ptr, rows);
    delete[] rows;
    png_write_end(png_ptr, info_ptr);
    png_free(png_ptr, palette);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(png_file);
    return 1;
}

img_info readPngImageFile(const char *file_name) {
    struct img_info _result_img_info{};
    //读取文件
    FILE *fp = fopen(file_name, "rb");
    if (fp == nullptr) {
        fclose(fp);
        return _result_img_info;
    }
    //创建用于读取的PNG结构，并分配所需的任何内存。
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png == nullptr) {
        fclose(fp);
        return _result_img_info;
    }
    //为应用程序的信息结构分配内存。
    png_infop info = png_create_info_struct(png);
    if (info == nullptr) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        fclose(fp);
        return _result_img_info;
    }
    //异常处理跳转
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, nullptr);
        fclose(fp);
        return _result_img_info;
    }
    //初始化png文件IO流
    png_init_io(png, fp);
    //不进行偏移
    png_set_sig_bytes(png, 0);
    //读取内容
    png_read_png(png, info, (PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND),
                 nullptr);
    int png_width = png_get_image_width(png, info);
    int png_height = png_get_image_height(png, info);
    //检查文件头数据块
    png_get_IHDR(png, info, nullptr, nullptr, nullptr,
                 nullptr, nullptr, nullptr, nullptr);
    unsigned int row_bytes = png_get_rowbytes(png, info);
    int rgb_size = (int) row_bytes * png_height;
    _result_img_info.bytes = (unsigned char *) malloc(sizeof(char) * rgb_size);    // 分配总内存
    png_bytepp rows = png_get_rows(png, info);
    _result_img_info.bit_depth = png_get_bit_depth(png, info);
    _result_img_info.color_type = png_get_color_type(png, info);
    unsigned char *tmp_buffer;
    tmp_buffer = _result_img_info.bytes;
    for (int i = 0; i < png_height; ++i) {
        memcpy(tmp_buffer, rows[i], row_bytes);
        tmp_buffer += row_bytes;
    }
    png_destroy_read_struct(&png, &info, nullptr);
    fclose(fp);
    _result_img_info.width = png_width;
    _result_img_info.height = png_height;
    return _result_img_info;
}

unsigned char *changeSize(int target_width, int target_height, int image_components,
                          unsigned char *source_buffer, int origin_width, int origin_height) {
    int sw = origin_width - 1, sh = origin_height - 1, dw = target_width - 1, dh =
            target_height - 1;
    int B, N, x, y;
    unsigned char *pLinePrev, *pLineNext;
    unsigned char *result_buffer = new unsigned char[target_width * target_height *
                                                     image_components];
    unsigned char *tmp;
    unsigned char *pA, *pB, *pC, *pD;
    for (int i = 0; i <= dh; ++i) {
        tmp = result_buffer + i * target_width * image_components;
        y = i * sh / dh;
        N = dh - i * sh % dh;
        pLinePrev = source_buffer + (y++) * origin_width * image_components;
        pLineNext = (N == dh) ? pLinePrev : source_buffer + y * origin_width * image_components;
        for (int j = 0; j <= dw; ++j) {
            x = j * sw / dw * image_components;
            B = dw - j * sw % dw;
            pA = pLinePrev + x;
            pB = pA + image_components;
            pC = pLineNext + x;
            pD = pC + image_components;
            if (B == dw) {
                pB = pA;
                pD = pC;
            }
            for (int k = 0; k < image_components; ++k) {
                *tmp++ = (unsigned char) (int) (
                        (B * N * (*pA++ - *pB - *pC + *pD) + dw * N * *pB++
                         + dh * B * *pC++ + (dw * dh - dh * B - dw * N) * *pD++
                         + dw * dh / 2) / (dw * dh));
            }
        }
    }
    return result_buffer;
}

/**
 *
 * @param source 源文件全路径
 * @param target 生成文件全路径
 * @param target_width 输出图片宽度，<=0 则使用源图片宽度
 * @param target_height 输出图片高度，<=0 则使用源图片高度
 * @param quality 输出图片质量（1~100）
 * @param keep_proportion 保持图片缩放比例，仅当target_width或target_height大于0时生效
 * @return 1--成功；-11--读取图片失败，-21--写入图片参数异常，-22--创建写入文件异常，-29--写入过程异常
 */
jint compressPicture(JNIEnv *env, jclass _, jstring source, jstring target, jint target_width,
                     jint target_height, jint quality, jboolean keep_proportion) {
    const char *source_path = env->GetStringUTFChars(source, nullptr);
    const char *target_path = env->GetStringUTFChars(target, nullptr);
    img_info source_info = readPngImageFile(source_path);
    if (source_info.bytes == nullptr || source_info.height == 0 || source_info.width == 0) {
        return -11;
    }
    LOGE("after read===>width=%d;;height=%d;;bit_depth=%d;;color_type=%d", source_info.width,
         source_info.height, source_info.bit_depth, source_info.color_type);
    unsigned char *source_buffer;
    int out_width;
    int out_height;
    if (target_width > 0 || target_height > 0) {
        if (target_width <= 0) {
            target_width = source_info.width;
        }
        if (target_height <= 0) {
            target_height = source_info.height;
        }
        if (keep_proportion) {
            float source_wh = ((float) source_info.width) / ((float) source_info.height);
            float target_wh = ((float) target_width) / ((float) target_height);
            if (target_wh > source_wh) {
                target_width = target_height * source_wh;
            } else if (target_wh < source_wh) {
                target_height = target_width / source_wh;
            }
        }
        //修改图片尺寸
        source_buffer = changeSize(target_width, target_height, 3, source_info.bytes,
                                   source_info.width, source_info.height);
        free(source_info.bytes);
        out_width = target_width;
        out_height = target_height;
    } else {
        source_buffer = source_info.bytes;
        out_width = source_info.width;
        out_height = source_info.height;
    }
    int result = writePngImageFile(target_path, source_buffer, out_width, out_height,
                                   source_info.bit_depth, source_info.color_type);
    free(source_buffer);
    return result;
}

jstring testLink(JNIEnv *env, jclass _) {
    return env->NewStringUTF("link success");
}

//TODO 对接实现的java class以及方法列表
static const char *CLASS_NAME = "com/zpf/pnglib/CompressUtil";
static JNINativeMethod nativeMethods[] = {
        {"testLink", "()Ljava/lang/String;",                        (void *) testLink},
        {"compress", "(Ljava/lang/String;Ljava/lang/String;IIIZ)I", (void *) compressPicture},
};

//回调函数
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    //获取JNIEnv
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    if (env == nullptr) {
        return -1;
    }
    jclass clazz;
    //找到声明native方法的类
    clazz = env->FindClass(CLASS_NAME);
    //注册函数 参数：java类 所要注册的函数数组 注册函数的个数
    int n = sizeof(nativeMethods) / sizeof(nativeMethods[0]);
    if ((env->RegisterNatives(clazz, nativeMethods, n)) < 0) {
        return -1;
    }
    //返回jni 的版本
    return JNI_VERSION_1_6;
}