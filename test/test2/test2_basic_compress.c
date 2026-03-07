/*
 * test2: 基础压缩/解压缩功能演示
 *
 * 本示例演示zlib最基本的compress和uncompress函数的使用方法
 * 适用于小块数据的压缩和解压缩场景
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* 打印十六进制数据 */
void print_hex(const char *label, const unsigned char *data, size_t len) {
    printf("%s [%zu bytes]: ", label, len);
    for (size_t i = 0; i < len && i < 32; i++) {
        printf("%02x ", data[i]);
    }
    if (len > 32) printf("...");
    printf("\n");
}

int main(void) {
    /* 原始数据 */
    const char *original = "Hello, World! This is a test string for zlib compression. "
                           " zlib is a widely used data compression library.";
    size_t original_len = strlen(original) + 1; /* 包含null终止符 */

    printf("========== 基础压缩/解压缩演示 ==========\n\n");

    printf("[1] 原始数据\n");
    printf("    内容: %s\n", original);
    printf("    长度: %zu 字节\n\n", original_len);

    /* 计算压缩缓冲区所需大小 */
    uLongf compressed_len = compressBound(original_len);
    unsigned char *compressed = (unsigned char *)malloc(compressed_len);
    if (!compressed) {
        fprintf(stderr, "错误: 无法分配压缩缓冲区\n");
        return 1;
    }

    /* 压缩数据 */
    printf("[2] 压缩数据\n");
    int ret = compress(compressed, &compressed_len,
                       (const Bytef *)original, original_len);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 压缩失败, 错误码: %d\n", ret);
        free(compressed);
        return 1;
    }

    printf("    压缩成功!\n");
    printf("    压缩后长度: %lu 字节\n", compressed_len);
    printf("    压缩率: %.2f%%\n", 100.0 * (1.0 - (double)compressed_len / original_len));
    print_hex("压缩数据", compressed, compressed_len);
    printf("\n");

    /* 分配解压缩缓冲区 */
    unsigned char *decompressed = (unsigned char *)malloc(original_len);
    if (!decompressed) {
        fprintf(stderr, "错误: 无法分配解压缩缓冲区\n");
        free(compressed);
        return 1;
    }

    /* 解压缩数据 */
    printf("[3] 解压缩数据\n");
    uLongf decompressed_len = original_len;
    ret = uncompress(decompressed, &decompressed_len,
                     compressed, compressed_len);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 解压缩失败, 错误码: %d\n", ret);
        free(compressed);
        free(decompressed);
        return 1;
    }

    printf("    解压缩成功!\n");
    printf("    解压缩后长度: %lu 字节\n", decompressed_len);
    printf("    内容: %s\n\n", (char *)decompressed);

    /* 验证数据完整性 */
    printf("[4] 数据完整性验证\n");
    if (decompressed_len == original_len &&
        memcmp(original, decompressed, original_len) == 0) {
        printf("    ✓ 数据验证通过! 原始数据和解压缩数据完全一致\n");
    } else {
        printf("    ✗ 数据验证失败! 数据不一致\n");
    }

    /* 清理资源 */
    free(compressed);
    free(decompressed);

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
