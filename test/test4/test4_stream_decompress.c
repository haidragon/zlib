/*
 * test4: 流式解压缩功能演示
 *
 * 本示例演示使用inflate进行流式解压缩
 * 适用于大文件或无法一次性加载到内存的压缩数据
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK_SIZE 16384  /* 16KB 块大小 */

/* 流式压缩函数 */
int stream_compress(const unsigned char *in_data, size_t in_len,
                   unsigned char **out_data, size_t *out_len) {
    z_stream stream;
    int ret;
    size_t have;
    unsigned char *out = NULL;
    size_t out_capacity = 0;
    size_t total_out = 0;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
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
        have = CHUNK_SIZE - stream.avail_out;
        total_out += have;
    } while (stream.avail_out == 0);

    deflateEnd(&stream);
    *out_data = out;
    *out_len = total_out;
    return Z_OK;
}

/* 流式解压缩函数 */
int stream_decompress(const unsigned char *in_data, size_t in_len,
                     unsigned char **out_data, size_t *out_len) {
    z_stream stream;
    int ret;
    size_t have;
    unsigned char *out = NULL;
    size_t out_capacity = 0;
    size_t total_out = 0;

    /* 初始化inflate流 */
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    ret = inflateInit(&stream);
    if (ret != Z_OK) {
        return ret;
    }

    /* 分配初始输出缓冲区 */
    out_capacity = CHUNK_SIZE;
    out = (unsigned char *)malloc(out_capacity);
    if (!out) {
        inflateEnd(&stream);
        return Z_MEM_ERROR;
    }

    /* 设置输入数据 */
    stream.avail_in = in_len;
    stream.next_in = (Bytef *)in_data;

    /* 解压缩循环 */
    do {
        /* 扩展输出缓冲区 */
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

        /* 设置输出缓冲区 */
        stream.avail_out = CHUNK_SIZE;
        stream.next_out = out + total_out;

        /* 执行解压缩 */
        ret = inflate(&stream, Z_NO_FLUSH);
        switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&stream);
                free(out);
                return ret;
        }

        have = CHUNK_SIZE - stream.avail_out;
        total_out += have;
    } while (stream.avail_out == 0);

    /* 清理 */
    inflateEnd(&stream);

    *out_data = out;
    *out_len = total_out;
    return Z_OK;
}

/* 生成测试数据 */
void generate_test_data(unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        data[i] = (unsigned char)('A' + (i % 26));
    }
}

int main(void) {
    printf("========== 流式解压缩演示 ==========\n\n");

    /* 生成测试数据 */
    size_t data_len = 100000;
    unsigned char *original = (unsigned char *)malloc(data_len);
    if (!original) {
        fprintf(stderr, "错误: 无法分配内存\n");
        return 1;
    }
    generate_test_data(original, data_len);

    printf("[1] 生成测试数据\n");
    printf("    原始数据长度: %zu 字节\n\n", data_len);

    /* 先压缩数据 */
    printf("[2] 压缩数据\n");
    unsigned char *compressed = NULL;
    size_t compressed_len = 0;

    int ret = stream_compress(original, data_len, &compressed, &compressed_len);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 压缩失败, 错误码: %d\n", ret);
        free(original);
        return 1;
    }
    printf("    压缩后长度: %zu 字节\n\n", compressed_len);

    /* 流式解压缩 */
    printf("[3] 执行流式解压缩\n");
    unsigned char *decompressed = NULL;
    size_t decompressed_len = 0;

    ret = stream_decompress(compressed, compressed_len, &decompressed, &decompressed_len);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 解压缩失败, 错误码: %d\n", ret);
        free(original);
        free(compressed);
        return 1;
    }

    printf("    解压缩成功!\n");
    printf("    解压缩后长度: %zu 字节\n\n", decompressed_len);

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
