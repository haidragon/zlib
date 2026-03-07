/*
 * test20: 文本数据优化演示
 *
 * 本示例演示如何优化文本数据的压缩
 * 包括预处理、字典使用等技巧
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <zlib.h>

/* 预处理: 统一换行符 */
void normalize_newlines(char *data, size_t *len) {
    size_t j = 0;
    for (size_t i = 0; i < *len; i++) {
        if (data[i] == '\r') {
            if (i + 1 < *len && data[i + 1] == '\n') {
                i++;  /* 跳过\n */
            }
            data[j++] = '\n';
        } else {
            data[j++] = data[i];
        }
    }
    *len = j;
}

/* 预处理: 移除多余空格 */
void remove_extra_whitespace(char *data, size_t *len) {
    size_t j = 0;
    int in_space = 0;

    for (size_t i = 0; i < *len; i++) {
        if (isspace((unsigned char)data[i])) {
            if (!in_space) {
                data[j++] = ' ';
                in_space = 1;
            }
        } else {
            data[j++] = data[i];
            in_space = 0;
        }
    }
    *len = j;
}

/* 压缩文本 */
int compress_text(const char *text, size_t len,
                 unsigned char **compressed, size_t *compressed_len,
                 int level) {
    uLongf bound = compressBound(len);
    *compressed = (unsigned char *)malloc(bound);
    if (!*compressed) return Z_MEM_ERROR;

    int ret = compress2(*compressed, compressed_len, (const Bytef *)text, len, level);
    if (ret != Z_OK) {
        free(*compressed);
        return ret;
    }
    return Z_OK;
}

/* 生成文本数据 */
void generate_lorem_ipsum(char *data, size_t len) {
    const char *lorem =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
        "Ut enim ad minim veniam, quis nostrud exercitation ullamco. "
        "Duis aute irure dolor in reprehenderit in voluptate velit esse. "
        "Excepteur sint occaecat cupidatat non proident, sunt in culpa. "
        "The quick brown fox jumps over the lazy dog. "
        "Pack my box with five dozen liquor jugs. ";
    size_t lorem_len = strlen(lorem);

    for (size_t i = 0; i < len; i++) {
        data[i] = lorem[i % lorem_len];
    }
}

void generate_json(char *data, size_t len) {
    const char *json_template =
        "{\"id\":%d,\"name\":\"%s\",\"value\":%f,\"status\":\"%s\","
        "\"description\":\"This is a JSON object for testing compression.\"},";
    size_t pos = 0;
    int id = 1;

    while (pos < len - 100) {
        pos += snprintf(data + pos, len - pos, json_template,
                       id, "Item", (double)id, "active");
        id++;
    }
}

int main(void) {
    printf("========== 文本数据优化演示 ==========\n\n");

    /* 测试1: 预处理效果 */
    printf("[测试1] 文本预处理效果\n");

    char mixed_newlines[1024];
    strcpy(mixed_newlines,
           "Line1\r\nLine2\rLine3\n\rLine4\n\nLine5");
    size_t len = strlen(mixed_newlines);

    printf("  原始文本 (混合换行符):\n");
    printf("    %s\n", mixed_newlines);
    printf("    长度: %zu\n", len);

    normalize_newlines(mixed_newlines, &len);
    mixed_newlines[len] = '\0';
    printf("  统一换行符后:\n");
    printf("    %s\n", mixed_newlines);
    printf("    长度: %zu\n", len);
    printf("\n");

    /* 测试2: 不同文本类型的压缩率 */
    printf("[测试2] 不同文本类型的压缩率\n");

    char lorem_text[10000];
    generate_lorem_ipsum(lorem_text, sizeof(lorem_text) - 1);

    unsigned char *compressed = NULL;
    size_t compressed_len = 0;

    printf("Lorem Ipsum文本:\n");
    compress_text(lorem_text, strlen(lorem_text), &compressed, &compressed_len, Z_DEFAULT_COMPRESSION);
    printf("  原始: %zu 字节, 压缩后: %zu 字节, 压缩率: %.2f%%\n\n",
           strlen(lorem_text), compressed_len,
           100.0 * (1.0 - (double)compressed_len / strlen(lorem_text)));
    free(compressed);

    char json_text[10000];
    generate_json(json_text, sizeof(json_text) - 1);

    printf("JSON数据:\n");
    compress_text(json_text, strlen(json_text), &compressed, &compressed_len, Z_DEFAULT_COMPRESSION);
    printf("  原始: %zu 字节, 压缩后: %zu 字节, 压缩率: %.2f%%\n\n",
           strlen(json_text), compressed_len,
           100.0 * (1.0 - (double)compressed_len / strlen(json_text)));
    free(compressed);

    /* 测试3: 压缩级别对文本的影响 */
    printf("[测试3] 不同压缩级别对文本的影响\n");

    generate_lorem_ipsum(lorem_text, 5000);
    printf("%-10s %-15s %-10s\n", "级别", "压缩后大小", "压缩率");
    printf("%-10s %-15s %-10s\n", "--------", "---------------", "----------");

    for (int level = 0; level <= 9; level++) {
        compress_text(lorem_text, strlen(lorem_text), &compressed, &compressed_len, level);
        double ratio = 100.0 * (1.0 - (double)compressed_len / strlen(lorem_text));
        printf("%-10d %-15zu %-10.2f%%\n", level, compressed_len, ratio);
        free(compressed);
    }

    printf("\n");

    /* 测试4: 空格优化 */
    printf("[测试4] 空格优化效果\n");

    char extra_spaces[1000] = "This  has    extra   spaces.  ";
    strcat(extra_spaces, "Multiple  spaces   between  words.  ");
    len = strlen(extra_spaces);

    printf("  原始文本 (%zu 字节): %.50s...\n", len, extra_spaces);
    compress_text(extra_spaces, len, &compressed, &compressed_len, Z_DEFAULT_COMPRESSION);
    printf("    压缩后: %zu 字节\n", compressed_len);
    free(compressed);

    remove_extra_whitespace(extra_spaces, &len);
    extra_spaces[len] = '\0';
    printf("  优化后 (%zu 字节): %.50s...\n", len, extra_spaces);
    compress_text(extra_spaces, len, &compressed, &compressed_len, Z_DEFAULT_COMPRESSION);
    printf("    压缩后: %zu 字节\n", compressed_len);
    free(compressed);

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
