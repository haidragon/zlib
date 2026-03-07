#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/**
 * 测试zlib压缩和解压功能
 */
int main(int argc, char *argv[]) {
    const char *source = "Hello, zlib! This is a test string for compression.";
    uLong sourceLen = strlen(source) + 1;  // 包含null终止符
    uLongf destLen = compressBound(sourceLen);
    Bytef *dest = malloc(destLen);
    Bytef *uncompressed = malloc(sourceLen);
    
    if (dest == NULL || uncompressed == NULL) {
        fprintf(stderr, "内存分配失败\n");
        free(dest);
        free(uncompressed);
        return 1;
    }
    
    printf("========== zlib压缩测试 ==========\n");
    printf("原始数据: %s\n", source);
    printf("原始数据大小: %lu 字节\n", sourceLen);
    printf("压缩缓冲区大小: %lu 字节\n", destLen);
    
    // 压缩
    int ret = compress(dest, &destLen, source, sourceLen);
    if (ret == Z_OK) {
        printf("\n[成功] 压缩完成！\n");
        printf("压缩后数据大小: %lu 字节\n", destLen);
        printf("压缩比: %.2f%%\n", (double)destLen / sourceLen * 100);
        printf("节省空间: %.2f%%\n", (1 - (double)destLen / sourceLen) * 100);
    } else {
        fprintf(stderr, "\n[错误] 压缩失败，错误码: %d\n", ret);
        free(dest);
        free(uncompressed);
        return 1;
    }
    
    // 解压
    uLongf uncompressedLen = sourceLen;
    ret = uncompress(uncompressed, &uncompressedLen, dest, destLen);
    if (ret == Z_OK) {
        printf("\n[成功] 解压完成！\n");
        printf("解压后数据大小: %lu 字节\n", uncompressedLen);
        printf("解压后数据: %s\n", (char *)uncompressed);
        
        // 验证数据一致性
        if (memcmp(source, uncompressed, sourceLen) == 0) {
            printf("\n[验证] 数据一致性检查通过！\n");
        } else {
            fprintf(stderr, "\n[错误] 数据不一致！\n");
        }
    } else {
        fprintf(stderr, "\n[错误] 解压失败，错误码: %d\n", ret);
        free(dest);
        free(uncompressed);
        return 1;
    }
    
    // 测试不同压缩级别
    printf("\n========== 不同压缩级别测试 ==========\n");
    int levels[] = {Z_NO_COMPRESSION, Z_BEST_SPEED, Z_DEFAULT_COMPRESSION, Z_BEST_COMPRESSION};
    const char *level_names[] = {"无压缩", "最快速度", "默认压缩", "最高压缩率"};
    
    for (int i = 0; i < 4; i++) {
        destLen = compressBound(sourceLen);
        ret = compress2(dest, &destLen, source, sourceLen, levels[i]);
        if (ret == Z_OK) {
            printf("[%s] 原始: %lu, 压缩后: %lu, 压缩比: %.2f%%\n",
                   level_names[i], sourceLen, destLen, (double)destLen / sourceLen * 100);
        }
    }
    
    printf("\n========== zlib版本信息 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    
    // 清理
    free(dest);
    free(uncompressed);
    
    printf("\n========== 测试完成 ==========\n");
    return 0;
}
