/*
 * test19: 图像数据压缩演示
 *
 * 本示例演示如何压缩不同类型的图像数据
 * PNG使用zlib进行压缩,这是zlib最重要的应用之一
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

typedef struct {
    int width;
    int height;
    int channels;
    unsigned char *data;
} Image;

/* 创建纯色图像 */
void create_solid_image(Image *img, int width, int height, int channels, unsigned char value) {
    img->width = width;
    img->height = height;
    img->channels = channels;
    img->data = (unsigned char *)malloc(width * height * channels);
    if (img->data) {
        memset(img->data, value, width * height * channels);
    }
}

/* 创建渐变图像 */
void create_gradient_image(Image *img, int width, int height, int channels) {
    img->width = width;
    img->height = height;
    img->channels = channels;
    img->data = (unsigned char *)malloc(width * height * channels);
    if (img->data) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                for (int c = 0; c < channels; c++) {
                    unsigned char value = (unsigned char)((x + y + c * 50) % 256);
                    img->data[(y * width + x) * channels + c] = value;
                }
            }
        }
    }
}

/* 创建随机噪声图像 */
void create_noise_image(Image *img, int width, int height, int channels) {
    img->width = width;
    img->height = height;
    img->channels = channels;
    img->data = (unsigned char *)malloc(width * height * channels);
    if (img->data) {
        for (int i = 0; i < width * height * channels; i++) {
            img->data[i] = (unsigned char)(rand() % 256);
        }
    }
}

/* 创建测试图像模式 */
void create_pattern_image(Image *img, int width, int height, int channels) {
    img->width = width;
    img->height = height;
    img->channels = channels;
    img->data = (unsigned char *)malloc(width * height * channels);
    if (img->data) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int pattern = (x / 32) % 2 == (y / 32) % 2 ? 0 : 255;
                for (int c = 0; c < channels; c++) {
                    img->data[(y * width + x) * channels + c] = (unsigned char)pattern;
                }
            }
        }
    }
}

/* PNG行过滤 (简化版) */
void apply_png_filter(unsigned char *data, int width, int height, int channels,
                     unsigned char *filtered) {
    size_t row_size = width * channels;

    for (int y = 0; y < height; y++) {
        /* 过滤类型字节 */
        filtered[y * (row_size + 1)] = 1;  /* Sub filter */

        /* 应用Sub过滤 */
        unsigned char *src_row = data + y * row_size;
        unsigned char *dst_row = filtered + y * (row_size + 1) + 1;

        for (int c = 0; c < channels; c++) {
            dst_row[c] = src_row[c];
        }
        for (size_t i = channels; i < row_size; i++) {
            dst_row[i] = src_row[i] - src_row[i - channels];
        }
    }
}

/* 压缩图像数据 */
int compress_image_data(const Image *img, unsigned char **compressed, size_t *compressed_len,
                        int use_filtering) {
    size_t data_size = img->width * img->height * img->channels;
    unsigned char *to_compress;

    if (use_filtering) {
        size_t filtered_size = (img->width * img->channels + 1) * img->height;
        to_compress = (unsigned char *)malloc(filtered_size);
        apply_png_filter(img->data, img->width, img->height, img->channels, to_compress);
        data_size = filtered_size;
    } else {
        to_compress = img->data;
    }

    uLongf bound = compressBound(data_size);
    *compressed = (unsigned char *)malloc(bound);
    if (!*compressed) {
        if (use_filtering) free(to_compress);
        return Z_MEM_ERROR;
    }

    int ret = compress(*compressed, compressed_len, to_compress, data_size);
    if (use_filtering) free(to_compress);

    return ret;
}

void print_image_info(const char *label, const Image *img, const unsigned char *compressed,
                     size_t compressed_len) {
    size_t original_size = img->width * img->height * img->channels;

    printf("%s:\n", label);
    printf("  分辨率: %dx%d\n", img->width, img->height);
    printf("  通道数: %d\n", img->channels);
    printf("  原始大小: %zu 字节 (%.2f KB)\n", original_size, original_size / 1024.0);
    printf("  压缩后大小: %zu 字节 (%.2f KB)\n", compressed_len, compressed_len / 1024.0);
    printf("  压缩率: %.2f%%\n\n", 100.0 * (1.0 - (double)compressed_len / original_size));
}

int main(void) {
    printf("========== 图像数据压缩演示 ==========\n");
    printf("PNG格式使用zlib进行压缩,以下测试模拟不同图像的压缩效果\n\n");

    Image img;
    unsigned char *compressed = NULL;
    size_t compressed_len = 0;

    /* 测试1: 纯色图像 (高压缩率) */
    printf("[测试1] 纯色图像 (高可压缩性)\n");
    create_solid_image(&img, 256, 256, 3, 128);
    compress_image_data(&img, &compressed, &compressed_len, 0);
    print_image_info("无过滤", &img, compressed, compressed_len);
    free(compressed);

    compress_image_data(&img, &compressed, &compressed_len, 1);
    print_image_info("PNG行过滤", &img, compressed, compressed_len);
    free(compressed);
    free(img.data);

    /* 测试2: 渐变图像 (中等压缩率) */
    printf("[测试2] 渐变图像 (中等可压缩性)\n");
    create_gradient_image(&img, 256, 256, 3);
    compress_image_data(&img, &compressed, &compressed_len, 0);
    print_image_info("无过滤", &img, compressed, compressed_len);
    free(compressed);
    free(img.data);

    /* 测试3: 噪声图像 (低压缩率) */
    printf("[测试3] 噪声图像 (低可压缩性)\n");
    create_noise_image(&img, 256, 256, 3);
    compress_image_data(&img, &compressed, &compressed_len, 0);
    print_image_info("无过滤", &img, compressed, compressed_len);
    free(compressed);
    free(img.data);

    /* 测试4: 测试图像 (使用PNG过滤) */
    printf("[测试4] 测试模式图像\n");
    create_pattern_image(&img, 256, 256, 1);
    compress_image_data(&img, &compressed, &compressed_len, 1);
    print_image_info("灰度+过滤", &img, compressed, compressed_len);
    free(compressed);

    create_pattern_image(&img, 256, 256, 3);
    compress_image_data(&img, &compressed, &compressed_len, 1);
    print_image_info("RGB+过滤", &img, compressed, compressed_len);
    free(compressed);
    free(img.data);

    printf("========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
