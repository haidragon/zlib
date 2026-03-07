/*
 * test5: 压缩级别对比测试
 *
 * 本示例演示zlib的不同压缩级别对压缩率和速度的影响
 * 压缩级别: 0(无压缩) 到 9(最大压缩)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zlib.h>

/* 压缩测试结果结构 */
typedef struct {
    int level;
    size_t original_size;
    size_t compressed_size;
    double compression_ratio;
    double compress_time_ms;
} CompressionResult;

/* 在指定级别下压缩数据 */
int compress_at_level(int level, const unsigned char *in_data, size_t in_len,
                     unsigned char *out_data, size_t *out_len,
                     double *time_ms) {
    z_stream stream;
    int ret;
    clock_t start, end;

    /* 初始化deflate流 */
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    ret = deflateInit(&stream, level);
    if (ret != Z_OK) {
        return ret;
    }

    stream.avail_in = in_len;
    stream.next_in = (Bytef *)in_data;
    stream.avail_out = *out_len;
    stream.next_out = out_data;

    /* 计时压缩 */
    start = clock();
    ret = deflate(&stream, Z_FINISH);
    end = clock();
    *time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

    deflateEnd(&stream);

    if (ret != Z_STREAM_END) {
        return Z_DATA_ERROR;
    }

    *out_len = stream.total_out;
    return Z_OK;
}

/* 生成可压缩的测试数据 */
void generate_compressible_data(unsigned char *data, size_t len) {
    /* 生成重复模式，便于压缩 */
    const char *pattern = "The quick brown fox jumps over the lazy dog. ";
    size_t pattern_len = strlen(pattern);

    for (size_t i = 0; i < len; i++) {
        data[i] = (unsigned char)pattern[i % pattern_len];
    }
}

int main(void) {
    printf("========== 压缩级别对比测试 ==========\n\n");

    /* 生成测试数据 */
    size_t data_len = 1000000;  /* 1MB */
    unsigned char *original = (unsigned char *)malloc(data_len);
    if (!original) {
        fprintf(stderr, "错误: 无法分配内存\n");
        return 1;
    }
    generate_compressible_data(original, data_len);

    printf("[测试数据]\n");
    printf("  原始大小: %zu 字节 (%.2f MB)\n\n", data_len, data_len / 1024.0 / 1024.0);

    /* 测试不同压缩级别 */
    CompressionResult results[10];
    uLongf max_compressed_len = compressBound(data_len);
    unsigned char *compressed = (unsigned char *)malloc(max_compressed_len);
    if (!compressed) {
        fprintf(stderr, "错误: 无法分配压缩缓冲区\n");
        free(original);
        return 1;
    }

    printf("%-8s %-15s %-15s %-12s %-12s\n",
           "级别", "压缩后大小", "压缩率", "时间(ms)", "相对速度");
    printf("%-8s %-15s %-15s %-12s %-12s\n",
           "--------", "---------------", "---------------", "------------", "------------");

    for (int level = 0; level <= 9; level++) {
        uLongf compressed_len = max_compressed_len;
        double time_ms = 0;

        int ret = compress_at_level(level, original, data_len,
                                   compressed, &compressed_len, &time_ms);
        if (ret != Z_OK) {
            fprintf(stderr, "错误: 级别%d压缩失败\n", level);
            continue;
        }

        /* 记录结果 */
        results[level].level = level;
        results[level].original_size = data_len;
        results[level].compressed_size = compressed_len;
        results[level].compression_ratio = 100.0 * (1.0 - (double)compressed_len / data_len);
        results[level].compress_time_ms = time_ms;

        /* 计算相对速度 (以级别0为基准) */
        double relative_speed = results[0].compress_time_ms > 0 ?
            results[0].compress_time_ms / time_ms : 1.0;

        const char *level_name;
        switch (level) {
            case 0: level_name = "0(无)"; break;
            case 1: level_name = "1(最快)"; break;
            case 9: level_name = "9(最大)"; break;
            default: {
                static char name[16];
                snprintf(name, sizeof(name), "%d", level);
                level_name = name;
            }
        }

        printf("%-8s %-15zu %-15.2f %-12.3f ", level_name,
               compressed_len, results[level].compression_ratio, time_ms);
        if (level > 0) {
            printf("%-12.2fx\n", relative_speed);
        } else {
            printf("%-12s\n", "基准");
        }
    }

    printf("\n[分析结论]\n");

    /* 找出最佳压缩率 */
    int best_level = 0;
    double best_ratio = results[0].compression_ratio;
    for (int i = 1; i <= 9; i++) {
        if (results[i].compression_ratio > best_ratio) {
            best_ratio = results[i].compression_ratio;
            best_level = i;
        }
    }
    printf("  最佳压缩率: 级别%d (%.2f%%)\n", best_level, best_ratio);

    /* 找出最快压缩 */
    int fastest_level = 0;
    double fastest_time = results[0].compress_time_ms;
    for (int i = 1; i <= 9; i++) {
        if (results[i].compress_time_ms < fastest_time) {
            fastest_time = results[i].compress_time_ms;
            fastest_level = i;
        }
    }
    printf("  最快压缩: 级别%d (%.3f ms)\n", fastest_level, fastest_time);

    /* 清理 */
    free(original);
    free(compressed);

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
