/*
 * test6: 大文件压缩演示
 *
 * 本示例演示如何压缩和解压缩大文件
 * 使用流式处理，避免一次性加载整个文件到内存
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK_SIZE 16384  /* 16KB 块大小 */

/* 压缩文件 */
int compress_file(FILE *source, FILE *dest, int level) {
    z_stream stream;
    int ret, flush;
    unsigned char in[CHUNK_SIZE];
    unsigned char out[CHUNK_SIZE];

    /* 初始化deflate流 */
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    ret = deflateInit(&stream, level);
    if (ret != Z_OK) {
        return ret;
    }

    /* 压缩循环 */
    do {
        stream.avail_in = fread(in, 1, CHUNK_SIZE, source);
        if (ferror(source)) {
            deflateEnd(&stream);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        stream.next_in = in;

        do {
            stream.avail_out = CHUNK_SIZE;
            stream.next_out = out;
            ret = deflate(&stream, flush);
            if (ret == Z_STREAM_ERROR) {
                deflateEnd(&stream);
                return ret;
            }
            size_t have = CHUNK_SIZE - stream.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                deflateEnd(&stream);
                return Z_ERRNO;
            }
        } while (stream.avail_out == 0);
    } while (flush != Z_FINISH);

    /* 清理 */
    deflateEnd(&stream);
    return Z_OK;
}

/* 解压缩文件 */
int decompress_file(FILE *source, FILE *dest) {
    z_stream stream;
    int ret;
    unsigned char in[CHUNK_SIZE];
    unsigned char out[CHUNK_SIZE];

    /* 初始化inflate流 */
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    ret = inflateInit(&stream);
    if (ret != Z_OK) {
        return ret;
    }

    /* 解压缩循环 */
    do {
        stream.avail_in = fread(in, 1, CHUNK_SIZE, source);
        if (ferror(source)) {
            inflateEnd(&stream);
            return Z_ERRNO;
        }
        if (stream.avail_in == 0) break;
        stream.next_in = in;

        do {
            stream.avail_out = CHUNK_SIZE;
            stream.next_out = out;
            ret = inflate(&stream, Z_NO_FLUSH);
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    inflateEnd(&stream);
                    return ret;
            }
            size_t have = CHUNK_SIZE - stream.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                inflateEnd(&stream);
                return Z_ERRNO;
            }
        } while (stream.avail_out == 0);
    } while (ret != Z_STREAM_END);

    /* 清理 */
    inflateEnd(&stream);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* 生成测试文件 */
void generate_test_file(const char *filename, size_t size) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "无法创建文件: %s\n", filename);
        return;
    }

    unsigned char buffer[CHUNK_SIZE];
    for (size_t i = 0; i < sizeof(buffer); i++) {
        buffer[i] = (unsigned char)('A' + (i % 26));
    }

    size_t written = 0;
    while (written < size) {
        size_t to_write = (size - written < sizeof(buffer)) ?
                          (size - written) : sizeof(buffer);
        fwrite(buffer, 1, to_write, fp);
        written += to_write;
    }

    fclose(fp);
}

/* 获取文件大小 */
long get_file_size(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return -1;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);
    return size;
}

int main(void) {
    const char *original_file = "test_original.dat";
    const char *compressed_file = "test_compressed.zlib";
    const char *decompressed_file = "test_decompressed.dat";

    printf("========== 大文件压缩演示 ==========\n\n");

    /* 生成测试文件 */
    printf("[1] 生成测试文件 (%s)\n", original_file);
    size_t file_size = 10 * 1024 * 1024;  /* 10MB */
    generate_test_file(original_file, file_size);
    long original_size = get_file_size(original_file);
    printf("    文件大小: %ld 字节 (%.2f MB)\n\n",
           original_size, original_size / 1024.0 / 1024.0);

    /* 压缩文件 */
    printf("[2] 压缩文件\n");
    FILE *source = fopen(original_file, "rb");
    FILE *dest = fopen(compressed_file, "wb");
    if (!source || !dest) {
        fprintf(stderr, "无法打开文件\n");
        if (source) fclose(source);
        if (dest) fclose(dest);
        return 1;
    }

    int ret = compress_file(source, dest, Z_DEFAULT_COMPRESSION);
    fclose(source);
    fclose(dest);

    if (ret != Z_OK) {
        fprintf(stderr, "错误: 压缩失败, 错误码: %d\n", ret);
        return 1;
    }

    long compressed_size = get_file_size(compressed_file);
    printf("    压缩成功!\n");
    printf("    压缩后大小: %ld 字节 (%.2f MB)\n",
           compressed_size, compressed_size / 1024.0 / 1024.0);
    printf("    压缩率: %.2f%%\n\n",
           100.0 * (1.0 - (double)compressed_size / original_size));

    /* 解压缩文件 */
    printf("[3] 解压缩文件\n");
    source = fopen(compressed_file, "rb");
    dest = fopen(decompressed_file, "wb");
    if (!source || !dest) {
        fprintf(stderr, "无法打开文件\n");
        if (source) fclose(source);
        if (dest) fclose(dest);
        return 1;
    }

    ret = decompress_file(source, dest);
    fclose(source);
    fclose(dest);

    if (ret != Z_OK) {
        fprintf(stderr, "错误: 解压缩失败, 错误码: %d\n", ret);
        return 1;
    }

    long decompressed_size = get_file_size(decompressed_file);
    printf("    解压缩成功!\n");
    printf("    解压缩后大小: %ld 字节 (%.2f MB)\n\n",
           decompressed_size, decompressed_size / 1024.0 / 1024.0);

    /* 验证文件 */
    printf("[4] 文件验证\n");
    if (original_size == decompressed_size) {
        printf("    ✓ 文件大小一致!\n");
    } else {
        printf("    ✗ 文件大小不一致!\n");
    }

    /* 清理测试文件 */
    remove(original_file);
    remove(compressed_file);
    remove(decompressed_file);
    printf("    已清理测试文件\n");

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
