#include <jni.h>
#include "jpeg/jpeglib.h"
#include <csetjmp>
#include <cstdio>
#include <malloc.h>
#include <cstring>

static jmp_buf jmp_buffer;

struct img_info {
    int width;
    int height;
    unsigned char *bytes;
};

void onErrorExit(j_common_ptr _) {
    longjmp(jmp_buffer, 1);
}

img_info readImageFile(const char *file_path) {
    struct jpeg_decompress_struct file_info{};
    struct jpeg_error_mgr err_info{};
    struct img_info _result_img_info{};
    FILE *file;
    file = fopen(file_path, "rb");
    if (file == nullptr) {
        return _result_img_info;
    }
    file_info.err = jpeg_std_error(&err_info);
    err_info.error_exit = onErrorExit;
    if (setjmp(jmp_buffer)) {
        jpeg_destroy_decompress(&file_info);
        fclose(file);
        return _result_img_info;
    }

    jpeg_create_decompress(&file_info);
    jpeg_stdio_src(&file_info, file);
    jpeg_read_header(&file_info, TRUE);
    file_info.out_color_space = JCS_RGB; //JCS_YCbCr;  // 设置输出格式
    jpeg_start_decompress(&file_info);

    JSAMPARRAY buffer;
    int row_stride;
    int rgb_size;
    row_stride = (int) file_info.output_width * file_info.output_components;
    rgb_size = row_stride * (int) file_info.output_height; // 总大小
    buffer = (*file_info.mem->alloc_sarray)((j_common_ptr) &file_info, JPOOL_IMAGE, row_stride, 1);
    _result_img_info.bytes = (unsigned char *) malloc(sizeof(char) * rgb_size);    // 分配总内存
    unsigned char *tmp_buffer;
    tmp_buffer = _result_img_info.bytes;
    while (file_info.output_scanline < file_info.output_height) {
        jpeg_read_scanlines(&file_info, buffer, 1);
        memcpy(tmp_buffer, buffer[0], row_stride);
        tmp_buffer += row_stride;
    }

    jpeg_finish_decompress(&file_info);
    fclose(file);
    jpeg_destroy_decompress(&file_info);
    _result_img_info.width = file_info.output_width;
    _result_img_info.height = file_info.output_height;
    return _result_img_info;
}

int writeImageFile(unsigned char *image_bytes, int image_width, int image_height,
                   int quality, const char *outPath) {
    if (image_bytes == nullptr || image_width <= 0 || image_height <= 0 ||
        quality < 0 || quality > 100 || outPath == nullptr) {
        return -21;
    }

    struct jpeg_compress_struct compress_info{};
    struct jpeg_error_mgr err_info{};
    /**
     * Step 1: 分配并初始化JPEG压缩对象
     *
     * 我们必须先设置错误处理程序，以防初始化步骤失败。
     * 虽然不太可能，但如果内存不足，可能会发生这种情况。
     * 这个例程将填充并返回在jpeg_compress_struct中对应jpeg_error_mgr的地址。
     */
    FILE *outfile;
    compress_info.err = jpeg_std_error(&err_info);
    err_info.error_exit = onErrorExit;
    if (setjmp(jmp_buffer)) {
        jpeg_destroy_compress(&compress_info);
        if (outfile != nullptr) {
            fclose(outfile);
        }
        return -29;
    }

    /** 现在初始化JPEG压缩对象。 */
    jpeg_create_compress(&compress_info);

    /**
     * Step 2: 指定数据输出目标 (例如：图片文件)
     * 注意：步骤2和3可以按任意顺序进行。
     * 这里我们使用库提供的代码将压缩数据发送到stdio流。您还可以编写自己的代码来执行其他操作。
     * 非常重要：如果你需要在设备上排序或写入二进制文件，请在使用fopen（）函数时使用“b”参数
     */
    if ((outfile = fopen(outPath, "wb")) == nullptr) {
        return -22;
    }
    jpeg_stdio_dest(&compress_info, outfile);

    /**
     * Step 3: 设定压缩参数
     * 以下4个参数必须填写：
     */
    compress_info.image_width = image_width;
    compress_info.image_height = image_height;
    compress_info.input_components = 3;
    compress_info.in_color_space = JCS_RGB;
    /**
     * 现在我们使用库中提供的默认配置参数
     *（必须在设定in_color_space参数之后调用，因为默认配置依赖数据源颜色控件参数）
     */
    jpeg_set_defaults(&compress_info);
    /**
     * 现在可以设定其他非默认参数
     */
    jpeg_set_quality(&compress_info, quality, TRUE);
    compress_info.optimize_coding = TRUE;//开启哈夫曼算法
    /**
     * Step 4: 开始压缩
     * 传参“TRUE”确保我们将编写一个完整的交换JPEG文件。除非你非常确定自己在做什么。
     */
    jpeg_start_compress(&compress_info, TRUE);

    /** Step 5: 循环扫描写入数据输出目标
     * 这里我们使用库的中的next_scanline函数循环逐行扫描；不过，如果您愿意，您可以传递更多扫描线。
     */
    JSAMPROW row_pointer[1];
    int row_stride;
    row_stride = image_width * 3;
    while (compress_info.next_scanline < compress_info.image_height) {
        row_pointer[0] = &image_bytes[compress_info.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&compress_info, row_pointer, 1);
    }
    /* Step 6: 压缩结束 */
    jpeg_finish_compress(&compress_info);
    fclose(outfile);
    /* Step 7: 释放压缩对象 */
    jpeg_destroy_compress(&compress_info);
    return 1;
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
 * @param remark 备注信息，暂时没有使用
 * @return 1--成功；-11--读取图片失败，-21--写入图片参数异常，-22--创建写入文件异常，-29--写入过程异常
 */
jint compressPicture(JNIEnv *env, jclass _, jstring source, jstring target, jint target_width,
                     jint target_height, jint quality,jstring remark) {
    const char *source_path = env->GetStringUTFChars(source, nullptr);
    const char *target_path = env->GetStringUTFChars(target, nullptr);
    img_info sourceInfo = readImageFile(source_path);
    if (sourceInfo.bytes == nullptr || sourceInfo.height == 0 || sourceInfo.width == 0) {
        return -11;
    }
    unsigned char *source_buffer;
    int outWidth;
    int outHeight;
    if (target_width > 0 || target_height > 0) {
        if (target_width <= 0) {
            target_width = sourceInfo.width;
        }
        if (target_height <= 0) {
            target_height = sourceInfo.height;
        }
        float source_wh = ((float) sourceInfo.width) / ((float) sourceInfo.height);
        float target_wh = ((float) target_width) / ((float) target_height);
        if (target_wh > source_wh) {
            target_width = target_height * source_wh;
        } else if (target_wh < source_wh) {
            target_height = target_width / source_wh;
        }
        //修改图片尺寸
        source_buffer = changeSize(target_width, target_height, 3, sourceInfo.bytes,
                                   sourceInfo.width, sourceInfo.height);
        free(sourceInfo.bytes);
        outWidth = target_width;
        outHeight = target_height;
    } else {
        source_buffer = sourceInfo.bytes;
        outWidth = sourceInfo.width;
        outHeight = sourceInfo.height;
    }
    int result = writeImageFile(source_buffer, outWidth, outHeight, quality, target_path);
    free(source_buffer);
    return result;
}

//TODO 对接实现的java class以及方法列表
static const char *CLASS_NAME = "com/zpf/compress/jpglib/CompressUtil";
static JNINativeMethod nativeMethods[] = {
        {"nativeCompress", "(Ljava/lang/String;Ljava/lang/String;IIILjava/lang/String;)I", (void *) compressPicture},
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