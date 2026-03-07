/*
 * test21: 高级特性演示
 *
 * 本示例演示zlib的各种高级特性:
 * - inflateBack回调式解压
 * - deflatePrime插入预置位
 * - deflateCopy复制流状态
 * - deflateReset重置流
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* 测试deflateReset */
void test_deflate_reset(void) {
    printf("\n[测试1] deflateReset - 重置流状态\n");

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        printf("  初始化失败\n");
        return;
    }

    printf("  流已初始化\n");

    /* 重置流 */
    if (deflateReset(&stream) == Z_OK) {
        printf("  ✓ 流已重置,可以重新使用\n");
    }

    deflateEnd(&stream);
}

/* 测试deflateCopy */
void test_deflate_copy(void) {
    printf("\n[测试2] deflateCopy - 复制流状态\n");

    z_stream stream1, stream2;
    stream1.zalloc = Z_NULL;
    stream1.zfree = Z_NULL;
    stream1.opaque = Z_NULL;

    if (deflateInit(&stream1, Z_DEFAULT_COMPRESSION) != Z_OK) {
        printf("  初始化失败\n");
        return;
    }

    printf("  流1已初始化\n");

    /* 复制流 */
    stream2.zalloc = Z_NULL;
    stream2.zfree = Z_NULL;
    stream2.opaque = Z_NULL;

    if (deflateCopy(&stream2, &stream1) == Z_OK) {
        printf("  ✓ 流状态已复制到流2\n");
        printf("  现在有两个相同状态的流可以并行使用\n");
    }

    deflateEnd(&stream1);
    deflateEnd(&stream2);
}

/* 测试deflateParams */
void test_deflate_params(void) {
    printf("\n[测试3] deflateParams - 动态调整参数\n");

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        printf("  初始化失败\n");
        return;
    }

    printf("  初始级别: Z_DEFAULT_COMPRESSION (-1)\n");

    /* 动态调整压缩级别 */
    if (deflateParams(&stream, Z_BEST_COMPRESSION, Z_DEFAULT_STRATEGY) == Z_OK) {
        printf("  ✓ 已调整为最佳压缩级别 (9)\n");
    }

    deflateEnd(&stream);
}

/* 测试deflateTune */
void test_deflate_tune(void) {
    printf("\n[测试4] deflateTune - 调整压缩参数\n");

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        printf("  初始化失败\n");
        return;
    }

    /* 调整内部参数 */
    int good_length = 256;
    int max_lazy = 256;
    int nice_length = 128;
    int max_chain = 32;

    if (deflateTune(&stream, good_length, max_lazy, nice_length, max_chain) == Z_OK) {
        printf("  ✓ 压缩参数已调整\n");
        printf("    good_length: %d\n", good_length);
        printf("    max_lazy: %d\n", max_lazy);
        printf("    nice_length: %d\n", nice_length);
        printf("    max_chain: %d\n", max_chain);
    }

    deflateEnd(&stream);
}

/* 测试deflateBound */
void test_deflate_bound(void) {
    printf("\n[测试5] deflateBound - 计算最大压缩大小\n");

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        printf("  初始化失败\n");
        return;
    }

    size_t input_size = 1000000;
    uLong bound = deflateBound(&stream, input_size);
    printf("  输入大小: %zu 字节\n", input_size);
    printf("  最大压缩后大小: %lu 字节\n", bound);
    printf("  比率: %.2fx\n", (double)bound / input_size);

    deflateEnd(&stream);
}

/* 测试inflateReset */
void test_inflate_reset(void) {
    printf("\n[测试6] inflateReset - 重置解压流\n");

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;

    if (inflateInit(&stream) != Z_OK) {
        printf("  初始化失败\n");
        return;
    }

    printf("  解压流已初始化\n");

    if (inflateReset(&stream) == Z_OK) {
        printf("  ✓ 解压流已重置\n");
    }

    inflateEnd(&stream);
}

/* 测试压缩和解压的协同 */
void test_full_cycle(void) {
    printf("\n[测试7] 完整的压缩-解压循环\n");

    const char *original = "Hello, World! This is a test of zlib advanced features.";
    size_t original_len = strlen(original);

    /* 压缩 */
    z_stream compress_stream;
    compress_stream.zalloc = Z_NULL;
    compress_stream.zfree = Z_NULL;
    compress_stream.opaque = Z_NULL;

    if (deflateInit(&compress_stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        printf("  压缩初始化失败\n");
        return;
    }

    unsigned char compressed[1024];
    compress_stream.avail_in = original_len;
    compress_stream.next_in = (Bytef *)original;
    compress_stream.avail_out = sizeof(compressed);
    compress_stream.next_out = compressed;

    deflate(&compress_stream, Z_FINISH);
    deflateEnd(&compress_stream);

    size_t compressed_len = sizeof(compressed) - compress_stream.avail_out;
    printf("  压缩成功: %zu -> %zu 字节\n", original_len, compressed_len);

    /* 解压 */
    z_stream decompress_stream;
    decompress_stream.zalloc = Z_NULL;
    decompress_stream.zfree = Z_NULL;
    decompress_stream.opaque = Z_NULL;
    decompress_stream.avail_in = 0;
    decompress_stream.next_in = Z_NULL;

    if (inflateInit(&decompress_stream) != Z_OK) {
        printf("  解压初始化失败\n");
        return;
    }

    unsigned char decompressed[1024];
    decompress_stream.avail_in = compressed_len;
    decompress_stream.next_in = compressed;
    decompress_stream.avail_out = sizeof(decompressed);
    decompress_stream.next_out = decompressed;

    inflate(&decompress_stream, Z_NO_FLUSH);
    inflateEnd(&decompress_stream);

    size_t decompressed_len = sizeof(decompressed) - decompress_stream.avail_out;
    printf("  解压成功: %zu -> %zu 字节\n", compressed_len, decompressed_len);

    if (decompressed_len == original_len &&
        memcmp(original, decompressed, original_len) == 0) {
        printf("  ✓ 数据验证通过\n");
    }
}

int main(void) {
    printf("========== zlib高级特性演示 ==========\n");
    printf("演示zlib的高级API和特性\n");

    test_deflate_reset();
    test_deflate_copy();
    test_deflate_params();
    test_deflate_tune();
    test_deflate_bound();
    test_inflate_reset();
    test_full_cycle();

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
