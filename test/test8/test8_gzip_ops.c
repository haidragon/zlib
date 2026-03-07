/*
 * test8: Gzip文件操作演示
 *
 * 本示例演示zlib的gzip文件读写API
 * gzopen, gzread, gzwrite, gzclose等函数的使用
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK_SIZE 16384

/* 写入gzip文件 */
int write_gzip_file(const char *filename, const char *content, int level) {
    gzFile file = gzopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "无法打开文件: %s\n", filename);
        return Z_ERRNO;
    }

    /* 设置压缩级别 */
    gzsetparams(file, level, Z_DEFAULT_STRATEGY);

    /* 写入内容 */
    size_t len = strlen(content);
    size_t written = gzwrite(file, content, len);

    gzclose(file);

    if (written != len) {
        fprintf(stderr, "写入错误\n");
        return Z_ERRNO;
    }

    return Z_OK;
}

/* 读取gzip文件 */
char *read_gzip_file(const char *filename) {
    gzFile file = gzopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "无法打开文件: %s\n", filename);
        return NULL;
    }

    /* 动态分配缓冲区 */
    size_t capacity = CHUNK_SIZE;
    char *buffer = (char *)malloc(capacity);
    if (!buffer) {
        gzclose(file);
        return NULL;
    }

    size_t total = 0;
    int bytes_read;

    while ((bytes_read = gzread(file, buffer + total, CHUNK_SIZE)) > 0) {
        total += bytes_read;
        if (total + CHUNK_SIZE > capacity) {
            capacity *= 2;
            char *new_buffer = (char *)realloc(buffer, capacity);
            if (!new_buffer) {
                free(buffer);
                gzclose(file);
                return NULL;
            }
            buffer = new_buffer;
        }
    }

    buffer[total] = '\0';
    gzclose(file);

    return buffer;
}

/* 获取gzip文件信息 */
void show_gzip_info(const char *filename) {
    gzFile file = gzopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "无法打开文件: %s\n", filename);
        return;
    }

    printf("    文件信息:\n");
    printf("      压缩后大小: %ld 字节\n", gztell(file));

    /* 读取全部以获取原始大小 */
    char buffer[CHUNK_SIZE];
    size_t total = 0;
    int bytes_read;
    while ((bytes_read = gzread(file, buffer, CHUNK_SIZE)) > 0) {
        total += bytes_read;
    }

    printf("      原始大小: %zu 字节\n", total);
    printf("      压缩率: %.2f%%\n", 100.0 * (1.0 - (double)gztell(file) / total));

    gzclose(file);
}

int main(void) {
    const char *test_file = "test_gzip.dat.gz";
    const char *test_content = "Hello, World!\n"
                               "This is a test file for zlib gzip operations.\n"
                               "Gzip is a widely used file format for compression.\n"
                               "It uses DEFLATE algorithm and includes headers.\n"
                               "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"
                               "Sed do eiusmod tempor incididunt ut labore et dolore.\n"
                               "Ut enim ad minim veniam, quis nostrud exercitation.\n"
                               "Duis aute irure dolor in reprehenderit in voluptate.\n";

    printf("========== Gzip文件操作演示 ==========\n\n");

    /* 写入gzip文件 */
    printf("[1] 写入gzip文件\n");
    printf("    文件名: %s\n", test_file);
    printf("    内容长度: %zu 字节\n", strlen(test_content));

    int ret = write_gzip_file(test_file, test_content, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 写入失败\n");
        return 1;
    }
    printf("    写入成功!\n\n");

    /* 显示文件信息 */
    printf("[2] 文件信息\n");
    show_gzip_info(test_file);
    printf("\n");

    /* 读取gzip文件 */
    printf("[3] 读取gzip文件\n");
    char *content = read_gzip_file(test_file);
    if (!content) {
        fprintf(stderr, "错误: 读取失败\n");
        return 1;
    }

    printf("    读取成功!\n");
    printf("    内容长度: %zu 字节\n", strlen(content));
    printf("    内容:\n");
    printf("    %s\n", content);

    /* 验证内容 */
    printf("[4] 内容验证\n");
    if (strcmp(test_content, content) == 0) {
        printf("    ✓ 内容验证通过!\n");
    } else {
        printf("    ✗ 内容验证失败!\n");
    }

    /* 清理 */
    free(content);
    remove(test_file);
    printf("\n    已清理测试文件\n");

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
