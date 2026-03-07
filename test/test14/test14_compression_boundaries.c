/*
 * test14: 压缩边界测试
 *
 * 本示例测试各种边界情况下的压缩行为
 * 包括空数据、极小数据、极大数据等
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* 测试压缩边界 */
void test_compression(const char *name, const unsigned char *data, size_t len) {
    printf("\n[%s]\n", name);
    printf("  输入大小: %zu 字节\n", len);

    uLongf bound = compressBound(len);
    printf("  最大压缩后大小: %lu 字节\n", bound);

    uLongf compressed_len = bound;
    unsigned char *compressed = (unsigned char *)malloc(bound);
    if (!compressed) {
        printf("  错误: 内存分配失败\n");
        return;
    }

    int ret = compress(compressed, &compressed_len, data, len);
    if (ret != Z_OK) {
        printf("  错误: 压缩失败, 错误码: %d\n", ret);
        free(compressed);
        return;
    }

    printf("  实际压缩后大小: %lu 字节\n", compressed_len);

    if (len > 0) {
        double ratio = 100.0 * (1.0 - (double)compressed_len / len);
        printf("  压缩率: %.2f%%\n", ratio);
        printf("  是否变大: %s\n",
               compressed_len > len ? "是 (正常)" : "否");
    } else {
        printf("  压缩率: N/A\n");
    }

    /* 验证解压缩 */
    if (len > 0) {
        unsigned char *decompressed = (unsigned char *)malloc(len);
        if (decompressed) {
            uLongf decompressed_len = len;
            ret = uncompress(decompressed, &decompressed_len,
                           compressed, compressed_len);
            if (ret == Z_OK && decompressed_len == len &&
                memcmp(data, decompressed, len) == 0) {
                printf("  ✓ 验证通过\n");
            } else {
                printf("  ✗ 验证失败\n");
            }
            free(decompressed);
        }
    }

    free(compressed);
}

int main(void) {
    printf("========== 压缩边界测试 ==========\n");
    printf("测试各种边界情况下的压缩行为\n");

    /* 测试1: 空数据 */
    printf("\n---------- 测试组1: 空数据 ----------");
    test_compression("空数据 (0字节)", (unsigned char *)"", 0);

    /* 测试2: 极小数据 */
    printf("\n---------- 测试组2: 极小数据 ----------");
    unsigned char byte1 = 'A';
    test_compression("单字节", &byte1, 1);

    unsigned char bytes2[2] = {'A', 'B'};
    test_compression("双字节", bytes2, 2);

    unsigned char bytes10[10] = "123456789";
    test_compression("10字节", bytes10, 10);

    /* 测试3: 随机数据 */
    printf("\n---------- 测试组3: 随机数据 ----------");
    unsigned char random[100];
    for (int i = 0; i < 100; i++) {
        random[i] = (unsigned char)(rand() % 256);
    }
    test_compression("随机数据 (100字节)", random, 100);

    /* 测试4: 重复数据 */
    printf("\n---------- 测试组4: 重复数据 ----------");
    unsigned char repeated[100];
    memset(repeated, 'A', 100);
    test_compression("全'A' (100字节)", repeated, 100);

    memset(repeated, 0, 100);
    test_compression("全0 (100字节)", repeated, 100);

    /* 测试5: 模式数据 */
    printf("\n---------- 测试组5: 模式数据 ----------");
    unsigned char pattern[256];
    for (int i = 0; i < 256; i++) {
        pattern[i] = (unsigned char)i;
    }
    test_compression("序列0-255 (256字节)", pattern, 256);

    for (int i = 0; i < 256; i++) {
        pattern[i] = (unsigned char)('A' + (i % 26));
    }
    test_compression("重复字母 (256字节)", pattern, 256);

    /* 测试6: 文本数据 */
    printf("\n---------- 测试组6: 文本数据 ----------");
    const char *text = "The quick brown fox jumps over the lazy dog. "
                      "This is a common test string for compression algorithms. "
                      "It contains common English words and patterns.";
    test_compression("英文文本", (unsigned char *)text, strlen(text));

    const char *numbers = "1234567890" "1234567890" "1234567890" "1234567890"
                         "1234567890" "1234567890" "1234567890" "1234567890";
    test_compression("重复数字", (unsigned char *)numbers, strlen(numbers));

    /* 测试7: 边界大小 */
    printf("\n---------- 测试组7: 边界大小 ----------");
    unsigned char boundary_data[1024];
    for (int i = 0; i < 1024; i++) {
        boundary_data[i] = (unsigned char)('A' + (i % 26));
    }

    test_compression("64字节", boundary_data, 64);
    test_compression("256字节", boundary_data, 256);
    test_compression("512字节", boundary_data, 512);
    test_compression("1024字节", boundary_data, 1024);

    /* 测试8: 压缩Bound准确性 */
    printf("\n---------- 测试组8: compressBound准确性 ----------");
    printf("\n测试compressBound返回的缓冲区大小是否足够:\n");

    size_t test_sizes[] = {10, 100, 1000, 10000, 100000};
    for (int i = 0; i < 5; i++) {
        size_t size = test_sizes[i];
        uLongf bound = compressBound(size);
        uLongf actual = bound;
        unsigned char *in = (unsigned char *)malloc(size);
        unsigned char *out = (unsigned char *)malloc(bound);

        if (in && out) {
            memset(in, 'A', size);
            int ret = compress(out, &actual, in, size);
            printf("  大小:%7zu bytes -> Bound:%7lu, 实际:%7lu, %s\n",
                   size, bound, actual,
                   (actual <= bound) ? "✓" : "✗");
        }

        free(in);
        free(out);
    }

    /* 测试9: 不同压缩级别的边界行为 */
    printf("\n---------- 测试组9: 不同压缩级别的边界行为 ----------");
    unsigned char test_data[1000];
    memset(test_data, 'X', 1000);

    printf("\n固定数据在不同压缩级别下的结果:\n");
    printf("%-10s %-15s %-10s\n", "级别", "压缩后大小", "压缩率");
    printf("%-10s %-15s %-10s\n", "--------", "---------------", "----------");

    for (int level = 0; level <= 9; level++) {
        uLongf len = compressBound(1000);
        unsigned char *out = (unsigned char *)malloc(len);
        if (out) {
            int ret = compress2(out, &len, test_data, 1000, level);
            if (ret == Z_OK) {
                double ratio = 100.0 * (1.0 - (double)len / 1000);
                printf("%-10d %-15lu %-10.2f%%\n", level, len, ratio);
            }
            free(out);
        }
    }

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
