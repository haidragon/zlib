/*
 * test11: 进度回调演示
 *
 * 本示例演示如何在压缩/解压缩过程中显示进度信息
 * 注意: zlib本身不直接支持进度回调,需要在外部实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK_SIZE 16384

/* 进度回调函数类型 */
typedef void (*ProgressCallback)(size_t processed, size_t total, const char *phase);

/* 带进度报告的压缩 */
int compress_with_progress(const unsigned char *in_data, size_t in_len,
                          unsigned char **out_data, size_t *out_len,
                          int level, ProgressCallback callback) {
    z_stream stream;
    unsigned char *out = NULL;
    size_t out_capacity = 0;
    size_t total_out = 0;
    size_t total_in = 0;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    int ret = deflateInit(&stream, level);
    if (ret != Z_OK) return ret;

    out_capacity = CHUNK_SIZE;
    out = (unsigned char *)malloc(out_capacity);
    if (!out) {
        deflateEnd(&stream);
        return Z_MEM_ERROR;
    }

    stream.avail_in = in_len;
    stream.next_in = (Bytef *)in_data;

    do {
        /* 扩展输出缓冲区 */
        if (total_out + CHUNK_SIZE > out_capacity) {
            out_capacity *= 2;
            unsigned char *new_out = (unsigned char *)realloc(out, out_capacity);
            if (!new_out) {
                free(out);
                deflateEnd(&stream);
                return Z_MEM_ERROR;
            }
            out = new_out;
        }

        stream.avail_out = CHUNK_SIZE;
        stream.next_out = out + total_out;

        ret = deflate(&stream, Z_FINISH);
        if (ret == Z_STREAM_ERROR) {
            free(out);
            deflateEnd(&stream);
            return ret;
        }

        /* 计算并报告进度 */
        size_t processed = in_len - stream.avail_in;
        if (processed > total_in) {
            total_in = processed;
            if (callback) {
                callback(total_in, in_len, "压缩中");
            }
        }

        total_out += CHUNK_SIZE - stream.avail_out;
    } while (stream.avail_out == 0);

    deflateEnd(&stream);

    *out_data = out;
    *out_len = total_out;
    return Z_OK;
}

/* 进度显示回调 */
void print_progress(size_t processed, size_t total, const char *phase) {
    int percent = (int)(processed * 100 / total);
    int bar_width = 40;
    int filled = (int)(bar_width * processed / total);

    printf("\r  [%s] [", phase);
    for (int i = 0; i < bar_width; i++) {
        printf("%c", i < filled ? '=' : ' ');
    }
    printf("] %3d%% (%zu/%zu bytes)", percent, processed, total);
    fflush(stdout);
}

/* 带进度报告的解压缩 */
int decompress_with_progress(const unsigned char *in_data, size_t in_len,
                            unsigned char **out_data, size_t *out_len,
                            ProgressCallback callback) {
    z_stream stream;
    unsigned char *out = NULL;
    size_t out_capacity = 0;
    size_t total_out = 0;
    size_t last_report = 0;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    int ret = inflateInit(&stream);
    if (ret != Z_OK) return ret;

    out_capacity = CHUNK_SIZE;
    out = (unsigned char *)malloc(out_capacity);
    if (!out) {
        inflateEnd(&stream);
        return Z_MEM_ERROR;
    }

    stream.avail_in = in_len;
    stream.next_in = (Bytef *)in_data;

    do {
        if (total_out + CHUNK_SIZE > out_capacity) {
            out_capacity *= 2;
            unsigned char *new_out = (unsigned char *)realloc(out, out_capacity);
            if (!new_out) {
                free(out);
                inflateEnd(&stream);
                return Z_MEM_ERROR;
            }
            out = new_out;
        }

        stream.avail_out = CHUNK_SIZE;
        stream.next_out = out + total_out;

        ret = inflate(&stream, Z_NO_FLUSH);
        switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                free(out);
                inflateEnd(&stream);
                return ret;
        }

        total_out += CHUNK_SIZE - stream.avail_out;

        /* 报告进度 (基于已处理的输入) */
        size_t processed = in_len - stream.avail_in;
        if (processed - last_report >= CHUNK_SIZE || ret == Z_STREAM_END) {
            if (callback) {
                callback(processed, in_len, "解压缩中");
            }
            last_report = processed;
        }
    } while (stream.avail_out == 0 && ret != Z_STREAM_END);

    inflateEnd(&stream);

    if (ret != Z_STREAM_END) {
        free(out);
        return Z_DATA_ERROR;
    }

    *out_data = out;
    *out_len = total_out;
    return Z_OK;
}

/* 生成可压缩的测试数据 */
void generate_test_data(unsigned char *data, size_t len) {
    const char *pattern = "The quick brown fox jumps over the lazy dog. ";
    size_t pattern_len = strlen(pattern);

    for (size_t i = 0; i < len; i++) {
        data[i] = (unsigned char)pattern[i % pattern_len];
    }
}

int main(void) {
    printf("========== 进度回调演示 ==========\n\n");

    /* 生成测试数据 */
    size_t data_len = 10 * 1024 * 1024;  /* 10MB */
    unsigned char *original = (unsigned char *)malloc(data_len);
    if (!original) {
        fprintf(stderr, "错误: 无法分配内存\n");
        return 1;
    }
    generate_test_data(original, data_len);

    printf("[1] 生成测试数据: %.2f MB\n\n", data_len / 1024.0 / 1024.0);

    /* 带进度的压缩 */
    printf("[2] 压缩数据 (显示进度):\n");
    unsigned char *compressed = NULL;
    size_t compressed_len = 0;

    int ret = compress_with_progress(original, data_len, &compressed, &compressed_len,
                                     Z_DEFAULT_COMPRESSION, print_progress);
    printf("\n");
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 压缩失败, 错误码: %d\n", ret);
        free(original);
        return 1;
    }

    printf("    压缩完成!\n");
    printf("    压缩后大小: %.2f MB\n", compressed_len / 1024.0 / 1024.0);
    printf("    压缩率: %.2f%%\n\n",
           100.0 * (1.0 - (double)compressed_len / data_len));

    /* 带进度的解压缩 */
    printf("[3] 解压缩数据 (显示进度):\n");
    unsigned char *decompressed = NULL;
    size_t decompressed_len = 0;

    ret = decompress_with_progress(compressed, compressed_len, &decompressed,
                                 &decompressed_len, print_progress);
    printf("\n");
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 解压缩失败, 错误码: %d\n", ret);
        free(original);
        free(compressed);
        return 1;
    }

    printf("    解压缩完成!\n");
    printf("    解压缩后大小: %.2f MB\n\n", decompressed_len / 1024.0 / 1024.0);

    /* 验证数据 */
    printf("[4] 数据验证\n");
    if (decompressed_len == data_len &&
        memcmp(original, decompressed, data_len) == 0) {
        printf("    ✓ 数据验证通过!\n");
    } else {
        printf("    ✗ 数据验证失败!\n");
    }

    /* 清理 */
    free(original);
    free(compressed);
    free(decompressed);

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
