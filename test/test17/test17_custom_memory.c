/*
 * test17: 自定义内存分配演示
 *
 * 本示例演示如何为zlib设置自定义内存分配函数
 * 适用于嵌入式系统或需要跟踪内存使用的场景
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* 内存统计 */
static size_t total_allocations = 0;
static size_t total_allocated_bytes = 0;
static size_t current_allocated_bytes = 0;

/* 自定义内存分配函数 */
void *custom_alloc(voidpf opaque, uInt items, uInt size) {
    size_t total = items * size;
    void *ptr = malloc(total);
    if (ptr) {
        total_allocations++;
        total_allocated_bytes += total;
        current_allocated_bytes += total;
    }
    return ptr;
}

/* 自定义内存释放函数 */
void custom_free(voidpf opaque, void *address) {
    if (address) {
        free(address);
    }
}

void reset_memory_stats(void) {
    total_allocations = 0;
    total_allocated_bytes = 0;
    current_allocated_bytes = 0;
}

void print_memory_stats(void) {
    printf("    内存统计:\n");
    printf("      分配次数: %zu\n", total_allocations);
    printf("      总分配字节: %zu\n", total_allocated_bytes);
    printf("      当前使用: %zu\n", current_allocated_bytes);
}

int main(void) {
    printf("========== 自定义内存分配演示 ==========\n\n");

    const char *test_data = "This is a test string for custom memory allocation. "
                           "We will track how much memory zlib uses.";
    size_t data_len = strlen(test_data);

    /* 测试1: 使用默认内存分配 */
    printf("[测试1] 使用默认内存分配\n");
    uLongf compressed_len = compressBound(data_len);
    unsigned char *compressed = (unsigned char *)malloc(compressed_len);
    unsigned char *decompressed = (unsigned char *)malloc(data_len);

    if (compressed && decompressed) {
        int ret = compress(compressed, &compressed_len, (Bytef *)test_data, data_len);
        if (ret == Z_OK) {
            uLongf decomp_len = data_len;
            ret = uncompress(decompressed, &decomp_len, compressed, compressed_len);
            if (ret == Z_OK) {
                printf("  压缩/解压缩成功\n");
            }
        }
    }
    printf("\n");

    /* 测试2: 使用自定义内存分配 */
    printf("[测试2] 使用自定义内存分配\n");
    reset_memory_stats();

    z_stream stream;
    stream.zalloc = custom_alloc;
    stream.zfree = custom_free;
    stream.opaque = Z_NULL;

    /* 压缩 */
    int ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    if (ret == Z_OK) {
        stream.avail_in = data_len;
        stream.next_in = (Bytef *)test_data;

        unsigned char *custom_compressed = (unsigned char *)malloc(compressed_len);
        stream.avail_out = compressed_len;
        stream.next_out = custom_compressed;

        ret = deflate(&stream, Z_FINISH);
        if (ret == Z_STREAM_END) {
            printf("  压缩成功\n");
            print_memory_stats();
        }

        deflateEnd(&stream);
        free(custom_compressed);
    }

    /* 测试3: 比较不同压缩级别的内存使用 */
    printf("\n[测试3] 不同压缩级别的内存使用对比\n");
    printf("%-10s %-15s %-15s\n", "级别", "分配次数", "总分配字节");
    printf("%-10s %-15s %-15s\n", "--------", "---------------", "---------------");

    for (int level = 1; level <= 9; level += 2) {
        reset_memory_stats();

        stream.zalloc = custom_alloc;
        stream.zfree = custom_free;
        stream.opaque = Z_NULL;

        if (deflateInit(&stream, level) == Z_OK) {
            unsigned char *temp = (unsigned char *)malloc(compressed_len);
            stream.avail_in = data_len;
            stream.next_in = (Bytef *)test_data;
            stream.avail_out = compressed_len;
            stream.next_out = temp;

            deflate(&stream, Z_FINISH);
            printf("%-10d %-15zu %-15zu\n",
                   level, total_allocations, total_allocated_bytes);

            deflateEnd(&stream);
            free(temp);
        }
    }

    /* 清理 */
    free(compressed);
    free(decompressed);

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
