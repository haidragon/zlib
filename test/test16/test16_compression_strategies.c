/*
 * test16: 压缩策略演示
 *
 * 本示例演示zlib的不同压缩策略:
 * - Z_DEFAULT_STRATEGY: 默认策略
 * - Z_FILTERED: 适合数据相关性强的数据
 * - Z_HUFFMAN_ONLY: 仅使用霍夫曼编码
 * - Z_RLE: 适合重复数据
 * - Z_FIXED: 使用固定的霍夫曼树
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK_SIZE 16384

typedef struct {
    int strategy;
    const char *name;
    size_t compressed_size;
    double compression_ratio;
} StrategyResult;

/* 使用指定策略压缩数据 */
int compress_with_strategy(const unsigned char *in_data, size_t in_len,
                          unsigned char **out_data, size_t *out_len,
                          int level, int strategy) {
    z_stream stream;
    unsigned char *out = NULL;
    size_t out_capacity = 0;
    size_t total_out = 0;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    int ret = deflateInit2(&stream, level, Z_DEFLATED,
                          15, 8, strategy);
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

/* 生成不同类型的数据 */
void generate_pattern_data(unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        data[i] = (unsigned char)(i % 256);
    }
}

void generate_repeated_data(unsigned char *data, size_t len) {
    memset(data, 'A', len);
}

void generate_filtered_data(unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        data[i] = (unsigned char)(i % 10 + '0');
    }
}

void generate_text_data(unsigned char *data, size_t len) {
    const char *text = "The quick brown fox jumps over the lazy dog. "
                      "This is a common test string. ";
    size_t text_len = strlen(text);
    for (size_t i = 0; i < len; i++) {
        data[i] = (unsigned char)text[i % text_len];
    }
}

int main(void) {
    printf("========== 压缩策略演示 ==========\n\n");

    StrategyResult strategies[] = {
        {Z_DEFAULT_STRATEGY, "Z_DEFAULT_STRATEGY", 0, 0},
        {Z_FILTERED, "Z_FILTERED", 0, 0},
        {Z_HUFFMAN_ONLY, "Z_HUFFMAN_ONLY", 0, 0},
        {Z_RLE, "Z_RLE", 0, 0},
        {Z_FIXED, "Z_FIXED", 0, 0}
    };
    int num_strategies = sizeof(strategies) / sizeof(strategies[0]);

    /* 测试1: 文本数据 */
    printf("[测试1] 文本数据\n");
    size_t data_len = 10000;
    unsigned char *data = (unsigned char *)malloc(data_len);
    generate_text_data(data, data_len);

    printf("  数据大小: %zu 字节\n\n", data_len);

    printf("%-25s %-15s %-10s\n", "策略", "压缩后大小", "压缩率");
    printf("%-25s %-15s %-10s\n",
           "-------------------------", "---------------", "----------");

    for (int i = 0; i < num_strategies; i++) {
        unsigned char *compressed = NULL;
        size_t compressed_len = 0;

        int ret = compress_with_strategy(data, data_len, &compressed, &compressed_len,
                                         Z_DEFAULT_COMPRESSION, strategies[i].strategy);
        if (ret == Z_OK) {
            strategies[i].compressed_size = compressed_len;
            strategies[i].compression_ratio = 100.0 * (1.0 - (double)compressed_len / data_len);

            printf("%-25s %-15zu %-10.2f%%\n",
                   strategies[i].name, compressed_len, strategies[i].compression_ratio);

            free(compressed);
        }
    }

    /* 找出最佳策略 */
    int best_strategy = 0;
    for (int i = 1; i < num_strategies; i++) {
        if (strategies[i].compression_ratio > strategies[best_strategy].compression_ratio) {
            best_strategy = i;
        }
    }
    printf("\n  最佳策略: %s (%.2f%%)\n\n",
           strategies[best_strategy].name, strategies[best_strategy].compression_ratio);

    /* 测试2: 重复数据 */
    printf("[测试2] 重复数据\n");
    generate_repeated_data(data, data_len);

    printf("%-25s %-15s %-10s\n", "策略", "压缩后大小", "压缩率");
    printf("%-25s %-15s %-10s\n",
           "-------------------------", "---------------", "----------");

    for (int i = 0; i < num_strategies; i++) {
        unsigned char *compressed = NULL;
        size_t compressed_len = 0;

        int ret = compress_with_strategy(data, data_len, &compressed, &compressed_len,
                                         Z_DEFAULT_COMPRESSION, strategies[i].strategy);
        if (ret == Z_OK) {
            printf("%-25s %-15zu %-10.2f%%\n",
                   strategies[i].name, compressed_len,
                   100.0 * (1.0 - (double)compressed_len / data_len));
            free(compressed);
        }
    }
    printf("\n");

    /* 测试3: 随机数据 */
    printf("[测试3] 随机数据\n");
    for (size_t i = 0; i < data_len; i++) {
        data[i] = (unsigned char)(rand() % 256);
    }

    printf("%-25s %-15s %-10s\n", "策略", "压缩后大小", "压缩率");
    printf("%-25s %-15s %-10s\n",
           "-------------------------", "---------------", "----------");

    for (int i = 0; i < num_strategies; i++) {
        unsigned char *compressed = NULL;
        size_t compressed_len = 0;

        int ret = compress_with_strategy(data, data_len, &compressed, &compressed_len,
                                         Z_DEFAULT_COMPRESSION, strategies[i].strategy);
        if (ret == Z_OK) {
            printf("%-25s %-15zu %-10.2f%%\n",
                   strategies[i].name, compressed_len,
                   100.0 * (1.0 - (double)compressed_len / data_len));
            free(compressed);
        }
    }

    free(data);

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
