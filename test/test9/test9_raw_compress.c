/*
 * test9: 原始数据压缩演示
 *
 * 本示例演示压缩不带任何头部或尾部的原始数据
 * 使用deflateInit2的windowBits参数控制
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK_SIZE 16384

/* 原始数据压缩 (无头部/尾部) */
int compress_raw(const unsigned char *in_data, size_t in_len,
                unsigned char **out_data, size_t *out_len, int level) {
    z_stream stream;
    unsigned char *out = NULL;
    size_t out_capacity = 0;
    size_t total_out = 0;

    /* 初始化deflate流, windowBits = -15 表示原始数据 */
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    int ret = deflateInit2(&stream, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        return ret;
    }

    /* 分配输出缓冲区 */
    out_capacity = CHUNK_SIZE;
    out = (unsigned char *)malloc(out_capacity);
    if (!out) {
        deflateEnd(&stream);
        return Z_MEM_ERROR;
    }

    stream.avail_in = in_len;
    stream.next_in = (Bytef *)in_data;

    /* 压缩循环 */
    do {
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

        total_out += CHUNK_SIZE - stream.avail_out;
    } while (stream.avail_out == 0);

    deflateEnd(&stream);

    *out_data = out;
    *out_len = total_out;
    return Z_OK;
}

/* 原始数据解压缩 */
int decompress_raw(const unsigned char *in_data, size_t in_len,
                  unsigned char **out_data, size_t expected_len) {
    z_stream stream;
    unsigned char *out = NULL;
    size_t out_capacity = 0;
    size_t total_out = 0;

    /* 初始化inflate流, windowBits = -15 表示原始数据 */
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    int ret = inflateInit2(&stream, -15);
    if (ret != Z_OK) {
        return ret;
    }

    /* 分配输出缓冲区 */
    out_capacity = expected_len > CHUNK_SIZE ? expected_len : CHUNK_SIZE;
    out = (unsigned char *)malloc(out_capacity);
    if (!out) {
        inflateEnd(&stream);
        return Z_MEM_ERROR;
    }

    stream.avail_in = in_len;
    stream.next_in = (Bytef *)in_data;

    /* 解压缩循环 */
    do {
        stream.avail_out = out_capacity - total_out;
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
    } while (stream.avail_out == 0 && ret != Z_STREAM_END);

    inflateEnd(&stream);

    if (ret != Z_STREAM_END) {
        free(out);
        return Z_DATA_ERROR;
    }

    *out_data = out;
    return Z_OK;
}

int main(void) {
    const char *test_data = "This is raw data compression test. "
                           "No headers or trailers are added. "
                           "Only pure DEFLATE compressed data. "
                           "This is useful for embedded systems "
                           "or custom protocols where you control "
                           "the format. The data should be "
                           "compressible to show good ratio. "
                           "Repeating patterns help compression. "
                           "The quick brown fox jumps over lazy dog.";

    printf("========== 原始数据压缩演示 ==========\n\n");

    size_t original_len = strlen(test_data);
    printf("[1] 原始数据\n");
    printf("    长度: %zu 字节\n", original_len);
    printf("    内容: %s\n\n", test_data);

    /* 原始压缩 */
    printf("[2] 原始数据压缩 (无头部/尾部)\n");
    unsigned char *compressed = NULL;
    size_t compressed_len = 0;

    int ret = compress_raw((unsigned char *)test_data, original_len,
                           &compressed, &compressed_len, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 压缩失败, 错误码: %d\n", ret);
        return 1;
    }

    printf("    压缩成功!\n");
    printf("    压缩后长度: %zu 字节\n", compressed_len);
    printf("    压缩率: %.2f%%\n\n",
           100.0 * (1.0 - (double)compressed_len / original_len));

    /* 原始解压缩 */
    printf("[3] 原始数据解压缩\n");
    unsigned char *decompressed = NULL;

    ret = decompress_raw(compressed, compressed_len, &decompressed, original_len);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 解压缩失败, 错误码: %d\n", ret);
        free(compressed);
        return 1;
    }

    printf("    解压缩成功!\n");
    printf("    解压缩后长度: %zu 字节\n", original_len);
    printf("    内容: %s\n\n", decompressed);

    /* 验证 */
    printf("[4] 数据验证\n");
    if (strlen((char *)decompressed) == original_len &&
        memcmp(test_data, decompressed, original_len) == 0) {
        printf("    ✓ 数据验证通过!\n");
    } else {
        printf("    ✗ 数据验证失败!\n");
    }

    /* 对比普通压缩 */
    printf("\n[5] 对比: 普通压缩 (带zlib头部)\n");
    uLongf zlib_compressed_len = compressBound(original_len);
    unsigned char *zlib_compressed = (unsigned char *)malloc(zlib_compressed_len);

    ret = compress(zlib_compressed, &zlib_compressed_len,
                   (Bytef *)test_data, original_len);
    if (ret == Z_OK) {
        printf("    zlib格式压缩后长度: %lu 字节\n", zlib_compressed_len);
        printf("    原始格式压缩后长度: %zu 字节\n", compressed_len);
        printf("    差异: %ld 字节 (%.2f%%)\n",
               (long)(zlib_compressed_len - compressed_len),
               100.0 * (zlib_compressed_len - compressed_len) / zlib_compressed_len);
        free(zlib_compressed);
    }

    /* 清理 */
    free(compressed);
    free(decompressed);

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
