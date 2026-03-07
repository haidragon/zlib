/*
 * test13: 部分解压缩演示
 *
 * 本示例演示如何只解压缩数据的一部分
 * 适用于只需要访问部分数据的场景
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK_SIZE 16384

/* 压缩数据 */
int compress_data(const unsigned char *in_data, size_t in_len,
                 unsigned char **out_data, size_t *out_len) {
    z_stream stream;
    unsigned char *out = NULL;
    size_t out_capacity = 0;
    size_t total_out = 0;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    int ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
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

/* 部分解压缩 - 只解压缩前N个字节 */
int partial_decompress(const unsigned char *in_data, size_t in_len,
                       unsigned char **out_data, size_t *out_len,
                       size_t max_bytes) {
    z_stream stream;
    unsigned char *out = NULL;
    size_t out_capacity = 0;
    size_t total_out = 0;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    int ret = inflateInit(&stream);
    if (ret != Z_OK) return ret;

    out_capacity = (max_bytes < CHUNK_SIZE) ? CHUNK_SIZE : max_bytes;
    out = (unsigned char *)malloc(out_capacity);
    if (!out) {
        inflateEnd(&stream);
        return Z_MEM_ERROR;
    }

    stream.avail_in = in_len;
    stream.next_in = (Bytef *)in_data;

    /* 解压缩直到达到最大字节数或流结束 */
    while (total_out < max_bytes) {
        stream.avail_out = (max_bytes - total_out < CHUNK_SIZE) ?
                           (max_bytes - total_out) : CHUNK_SIZE;
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

        size_t produced = CHUNK_SIZE - stream.avail_out;
        total_out += produced;

        if (ret == Z_STREAM_END || produced == 0) {
            break;
        }
    }

    inflateEnd(&stream);

    *out_data = out;
    *out_len = total_out;
    return Z_OK;
}

int main(void) {
    printf("========== 部分解压缩演示 ==========\n\n");

    /* 创建测试数据 */
    char original_data[4096];
    sprintf(original_data,
            "Line 1: This is the first line of data.\n"
            "Line 2: This is the second line.\n"
            "Line 3: Third line with more content.\n"
            "Line 4: Fourth line continues.\n"
            "Line 5: Fifth line example.\n"
            "Line 6: Sixth line data.\n"
            "Line 7: Seventh line information.\n"
            "Line 8: Eighth line sample.\n"
            "Line 9: Ninth line test data.\n"
            "Line 10: Tenth line and more.\n");

    size_t original_len = strlen(original_data);
    printf("[1] 原始数据\n");
    printf("    总长度: %zu 字节\n\n", original_len);

    /* 压缩数据 */
    printf("[2] 压缩数据\n");
    unsigned char *compressed = NULL;
    size_t compressed_len = 0;

    int ret = compress_data((unsigned char *)original_data, original_len,
                           &compressed, &compressed_len);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 压缩失败\n");
        return 1;
    }

    printf("    压缩后长度: %zu 字节\n\n", compressed_len);

    /* 测试1: 只解压缩前100字节 */
    printf("[3] 测试1: 部分解压缩 (前100字节)\n");
    unsigned char *partial = NULL;
    size_t partial_len = 0;

    ret = partial_decompress(compressed, compressed_len, &partial, &partial_len, 100);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 解压缩失败\n");
        free(compressed);
        return 1;
    }

    printf("    解压缩了: %zu 字节\n", partial_len);
    printf("    内容:\n");
    printf("    %.*s\n\n", (int)partial_len, (char *)partial);

    /* 测试2: 只解压缩前50字节 */
    printf("[4] 测试2: 部分解压缩 (前50字节)\n");
    free(partial);
    partial = NULL;
    partial_len = 0;

    ret = partial_decompress(compressed, compressed_len, &partial, &partial_len, 50);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 解压缩失败\n");
        free(compressed);
        return 1;
    }

    printf("    解压缩了: %zu 字节\n", partial_len);
    printf("    内容:\n");
    printf("    %.*s\n\n", (int)partial_len, (char *)partial);

    /* 验证部分数据 */
    printf("[5] 验证部分数据\n");
    if (memcmp(original_data, partial, partial_len) == 0) {
        printf("    ✓ 部分数据验证通过!\n");
    } else {
        printf("    ✗ 部分数据验证失败!\n");
    }

    /* 对比完整解压缩 */
    printf("\n[6] 对比: 完整解压缩\n");
    unsigned char *full = NULL;
    size_t full_len = original_len;

    ret = partial_decompress(compressed, compressed_len, &full, &full_len, full_len);
    if (ret == Z_OK) {
        printf("    完整解压缩: %zu 字节\n", full_len);
        if (full_len == original_len &&
            memcmp(original_data, full, original_len) == 0) {
            printf("    ✓ 完整数据验证通过!\n");
        }
        free(full);
    }

    /* 清理 */
    free(compressed);
    free(partial);

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
