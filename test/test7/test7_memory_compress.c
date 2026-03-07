/*
 * test7: 内存压缩演示
 *
 * 本示例演示如何在内存中进行压缩和解压缩
 * 适用于网络传输、缓存等场景
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* 压缩数据 */
int compress_memory(const unsigned char *in_data, size_t in_len,
                   unsigned char **out_data, size_t *out_len,
                   int level) {
    uLongf bound = compressBound(in_len);
    unsigned char *out = (unsigned char *)malloc(bound);
    if (!out) return Z_MEM_ERROR;

    uLongf actual_len = bound;
    int ret = compress2(out, &actual_len, in_data, in_len, level);
    if (ret != Z_OK) {
        free(out);
        return ret;
    }

    *out_data = out;
    *out_len = actual_len;
    return Z_OK;
}

/* 解压缩数据 */
int decompress_memory(const unsigned char *in_data, size_t in_len,
                     unsigned char **out_data, size_t out_len) {
    unsigned char *out = (unsigned char *)malloc(out_len);
    if (!out) return Z_MEM_ERROR;

    uLongf actual_out_len = out_len;
    int ret = uncompress(out, &actual_out_len, in_data, in_len);
    if (ret != Z_OK) {
        free(out);
        return ret;
    }

    *out_data = out;
    return Z_OK;
}

/* 创建测试数据 */
typedef struct {
    int id;
    char name[64];
    double value;
    char data[256];
} TestData;

void generate_test_data(TestData *data, int count) {
    for (int i = 0; i < count; i++) {
        data[i].id = i;
        snprintf(data[i].name, sizeof(data[i].name), "Item_%04d", i);
        data[i].value = (double)i * 1.5;
        memset(data[i].data, 'A' + (i % 26), sizeof(data[i].data));
    }
}

int main(void) {
    printf("========== 内存压缩演示 ==========\n\n");

    /* 创建测试数据 */
    const int data_count = 1000;
    TestData *original = (TestData *)malloc(sizeof(TestData) * data_count);
    if (!original) {
        fprintf(stderr, "错误: 无法分配内存\n");
        return 1;
    }
    generate_test_data(original, data_count);

    size_t original_size = sizeof(TestData) * data_count;
    printf("[1] 创建测试数据\n");
    printf("    数据项数量: %d\n", data_count);
    printf("    原始大小: %zu 字节 (%.2f KB)\n\n",
           original_size, original_size / 1024.0);

    /* 压缩数据 */
    printf("[2] 压缩数据\n");
    unsigned char *compressed = NULL;
    size_t compressed_len = 0;

    int ret = compress_memory((unsigned char *)original, original_size,
                             &compressed, &compressed_len,
                             Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 压缩失败, 错误码: %d\n", ret);
        free(original);
        return 1;
    }

    printf("    压缩成功!\n");
    printf("    压缩后大小: %zu 字节 (%.2f KB)\n",
           compressed_len, compressed_len / 1024.0);
    printf("    压缩率: %.2f%%\n",
           100.0 * (1.0 - (double)compressed_len / original_size));
    printf("    节省内存: %zu 字节 (%.2f KB)\n\n",
           original_size - compressed_len,
           (original_size - compressed_len) / 1024.0);

    /* 解压缩数据 */
    printf("[3] 解压缩数据\n");
    unsigned char *decompressed = NULL;

    ret = decompress_memory(compressed, compressed_len,
                           &decompressed, original_size);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 解压缩失败, 错误码: %d\n", ret);
        free(original);
        free(compressed);
        return 1;
    }

    printf("    解压缩成功!\n");
    printf("    解压缩后大小: %zu 字节\n\n", original_size);

    /* 验证数据 */
    printf("[4] 数据验证\n");
    TestData *decomp_data = (TestData *)decompressed;
    int match_count = 0;
    for (int i = 0; i < data_count; i++) {
        if (original[i].id == decomp_data[i].id &&
            strcmp(original[i].name, decomp_data[i].name) == 0 &&
            original[i].value == decomp_data[i].value) {
            match_count++;
        }
    }

    if (match_count == data_count) {
        printf("    ✓ 所有数据验证通过!\n");
    } else {
        printf("    ✗ 数据验证失败! 匹配: %d/%d\n", match_count, data_count);
    }

    /* 清理 */
    free(original);
    free(compressed);
    free(decompressed);

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
