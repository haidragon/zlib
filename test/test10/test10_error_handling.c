/*
 * test10: 错误处理演示
 *
 * 本示例演示zlib的各种错误情况及其处理方法
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* 错误码描述 */
const char *zlib_error_string(int err) {
    switch (err) {
        case Z_OK: return "Z_OK - 操作成功";
        case Z_STREAM_END: return "Z_STREAM_END - 流结束";
        case Z_NEED_DICT: return "Z_NEED_DICT - 需要字典";
        case Z_ERRNO: return "Z_ERRNO - 系统错误";
        case Z_STREAM_ERROR: return "Z_STREAM_ERROR - 流参数错误";
        case Z_DATA_ERROR: return "Z_DATA_ERROR - 数据损坏或不完整";
        case Z_MEM_ERROR: return "Z_MEM_ERROR - 内存不足";
        case Z_BUF_ERROR: return "Z_BUF_ERROR - 缓冲区错误";
        case Z_VERSION_ERROR: return "Z_VERSION_ERROR - 版本不兼容";
        default: return "未知错误";
    }
}

/* 测试1: 内存不足 */
void test_memory_error(void) {
    printf("\n[测试1] 内存不足错误\n");
    unsigned char *data = (unsigned char *)"Hello, World!";
    unsigned char compressed[1024];
    uLongf compressed_len = sizeof(compressed);

    /* 使用过大的级别 (无效) */
    int ret = compress2(compressed, &compressed_len, data, strlen((char *)data), 100);
    if (ret != Z_OK) {
        printf("    捕获错误: %s\n", zlib_error_string(ret));
    }
}

/* 测试2: 数据损坏 */
void test_data_error(void) {
    printf("\n[测试2] 数据损坏错误\n");
    const char *original = "Hello, World!";
    unsigned char compressed[1024];
    unsigned char decompressed[1024];
    uLongf compressed_len = sizeof(compressed);
    uLongf decompressed_len = sizeof(decompressed);

    /* 正常压缩 */
    int ret = compress(compressed, &compressed_len, (Bytef *)original, strlen(original) + 1);
    if (ret != Z_OK) {
        printf("    压缩失败: %s\n", zlib_error_string(ret));
        return;
    }

    /* 损坏压缩数据 */
    if (compressed_len > 0) {
        compressed[0] ^= 0xFF;  /* 翻转第一位 */
    }

    /* 尝试解压缩 */
    ret = uncompress(decompressed, &decompressed_len, compressed, compressed_len);
    if (ret != Z_OK) {
        printf("    捕获错误: %s\n", zlib_error_string(ret));
    }
}

/* 测试3: 缓冲区太小 */
void test_buffer_error(void) {
    printf("\n[测试3] 缓冲区太小错误\n");
    const char *original = "This is a test string that will be too large for the small buffer.";
    unsigned char compressed[4];  /* 故意设置太小 */
    uLongf compressed_len = sizeof(compressed);

    int ret = compress(compressed, &compressed_len, (Bytef *)original, strlen(original) + 1);
    if (ret != Z_OK) {
        printf("    捕获错误: %s\n", zlib_error_string(ret));
        printf("    所需大小: %lu 字节 (使用compressBound获取)\n",
               compressBound(strlen(original) + 1));
    }
}

/* 测试4: 流参数错误 */
void test_stream_error(void) {
    printf("\n[测试4] 流参数错误\n");
    z_stream stream;

    /* 初始化无效的压缩级别 */
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    int ret = deflateInit(&stream, -1);  /* 无效级别 */
    if (ret != Z_OK) {
        printf("    捕获错误: %s\n", zlib_error_string(ret));
    }
}

/* 测试5: 不完整的压缩数据 */
void test_incomplete_data(void) {
    printf("\n[测试5] 不完整的压缩数据\n");
    const char *original = "Hello, World! This is a test.";
    unsigned char compressed[1024];
    unsigned char decompressed[1024];
    uLongf compressed_len = sizeof(compressed);
    uLongf decompressed_len = sizeof(decompressed);

    /* 压缩 */
    int ret = compress(compressed, &compressed_len, (Bytef *)original, strlen(original) + 1);
    if (ret != Z_OK) {
        printf("    压缩失败: %s\n", zlib_error_string(ret));
        return;
    }

    /* 只提供部分数据 */
    ret = uncompress(decompressed, &decompressed_len, compressed, compressed_len / 2);
    if (ret != Z_OK) {
        printf("    捕获错误: %s\n", zlib_error_string(ret));
    }
}

/* 测试6: 版本错误 */
void test_version_error(void) {
    printf("\n[测试6] zlib版本检查\n");
    printf("    当前zlib版本: %s\n", zlibVersion());
    printf("    头文件版本: %s\n", ZLIB_VERSION);

    if (strcmp(zlibVersion(), ZLIB_VERSION) != 0) {
        printf("    警告: 版本不匹配!\n");
    } else {
        printf("    ✓ 版本匹配\n");
    }
}

/* 测试7: 正确的错误处理示例 */
void test_proper_error_handling(void) {
    printf("\n[测试7] 正确的错误处理示例\n");
    const char *data = "Test data for error handling demonstration.";
    uLongf compressed_len = compressBound(strlen(data) + 1);
    unsigned char *compressed = (unsigned char *)malloc(compressed_len);
    unsigned char *decompressed = (unsigned char *)malloc(strlen(data) + 1);
    uLongf decompressed_len = strlen(data) + 1;

    if (!compressed || !decompressed) {
        printf("    错误: 内存分配失败\n");
        free(compressed);
        free(decompressed);
        return;
    }

    /* 压缩并检查错误 */
    int ret = compress(compressed, &compressed_len, (Bytef *)data, strlen(data) + 1);
    if (ret != Z_OK) {
        printf("    压缩错误: %s\n", zlib_error_string(ret));
        free(compressed);
        free(decompressed);
        return;
    }

    /* 解压缩并检查错误 */
    ret = uncompress(decompressed, &decompressed_len, compressed, compressed_len);
    if (ret != Z_OK) {
        printf("    解压缩错误: %s\n", zlib_error_string(ret));
        free(compressed);
        free(decompressed);
        return;
    }

    printf("    ✓ 所有操作成功完成\n");

    free(compressed);
    free(decompressed);
}

int main(void) {
    printf("========== 错误处理演示 ==========\n");
    printf("演示各种错误情况及其处理方法\n");

    test_memory_error();
    test_data_error();
    test_buffer_error();
    test_stream_error();
    test_incomplete_data();
    test_version_error();
    test_proper_error_handling();

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
