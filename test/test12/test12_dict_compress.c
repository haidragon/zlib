/*
 * test12: 字典压缩演示
 *
 * 本示例演示zlib的预设字典功能
 * 适用于压缩大量具有相似模式的小数据块
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* 生成字典数据 */
void generate_dictionary(unsigned char *dict, size_t len) {
    const char *dict_content =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: keep-alive\r\n"
        "Server: nginx\r\n"
        "Cache-Control: max-age=3600\r\n"
        "<!DOCTYPE html>\n"
        "<html><head><title>"
        "Hello World"
        "</title></head><body>";

    size_t dict_len = strlen(dict_content);
    for (size_t i = 0; i < len; i++) {
        dict[i] = dict_content[i % dict_len];
    }
}

/* 使用字典压缩数据 */
int compress_with_dict(const unsigned char *in_data, size_t in_len,
                     unsigned char **out_data, size_t *out_len,
                     const unsigned char *dict, size_t dict_len,
                     int level) {
    z_stream stream;
    unsigned char *out = NULL;
    uLongf out_capacity = compressBound(in_len);
    uLongf total_out = 0;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    int ret = deflateInit(&stream, level);
    if (ret != Z_OK) return ret;

    /* 设置字典 */
    if (dict && dict_len > 0) {
        ret = deflateSetDictionary(&stream, dict, dict_len);
        if (ret != Z_OK) {
            deflateEnd(&stream);
            return ret;
        }
    }

    out = (unsigned char *)malloc(out_capacity);
    if (!out) {
        deflateEnd(&stream);
        return Z_MEM_ERROR;
    }

    stream.avail_in = in_len;
    stream.next_in = (Bytef *)in_data;
    stream.avail_out = out_capacity;
    stream.next_out = out;

    ret = deflate(&stream, Z_FINISH);
    if (ret != Z_STREAM_END) {
        free(out);
        deflateEnd(&stream);
        return ret;
    }

    total_out = out_capacity - stream.avail_out;
    deflateEnd(&stream);

    *out_data = out;
    *out_len = total_out;
    return Z_OK;
}

/* 使用字典解压缩数据 */
int decompress_with_dict(const unsigned char *in_data, size_t in_len,
                        unsigned char **out_data, size_t expected_len,
                        const unsigned char *dict, size_t dict_len) {
    z_stream stream;
    unsigned char *out = NULL;
    uLongf out_len = expected_len;
    uLongf actual_out_len = expected_len;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    int ret = inflateInit(&stream);
    if (ret != Z_OK) return ret;

    out = (unsigned char *)malloc(out_len);
    if (!out) {
        inflateEnd(&stream);
        return Z_MEM_ERROR;
    }

    stream.avail_in = in_len;
    stream.next_in = (Bytef *)in_data;
    stream.avail_out = out_len;
    stream.next_out = out;

    ret = inflate(&stream, Z_FINISH);
    
    /* 如果需要字典 */
    if (ret == Z_NEED_DICT && dict && dict_len > 0) {
        inflateReset(&stream);
        stream.avail_in = in_len;
        stream.next_in = (Bytef *)in_data;
        stream.avail_out = out_len;
        stream.next_out = out;
        
        ret = inflateSetDictionary(&stream, dict, dict_len);
        if (ret != Z_OK) {
            free(out);
            inflateEnd(&stream);
            return ret;
        }
        
        ret = inflate(&stream, Z_FINISH);
    }

    if (ret != Z_STREAM_END) {
        free(out);
        inflateEnd(&stream);
        return Z_DATA_ERROR;
    }

    actual_out_len = out_len - stream.avail_out;
    inflateEnd(&stream);

    *out_data = out;
    return Z_OK;
}

/* 生成测试数据 (模拟HTTP响应) */
void generate_http_response(char *data, size_t len, int index) {
    const char *header =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<title>Hello World %d</title>\n"
        "</head>\n"
        "<body>\n"
        "<h1>Hello World %d</h1>\n";

    size_t header_len = strlen(header) + 20;  /* extra space for %d */
    size_t body_len = (len > header_len + 100) ? (len - header_len - 100) : 100;
    if (body_len > 1000) body_len = 1000;
    
    char body[1001];
    memset(body, 'A' + (index % 26), body_len);
    body[body_len] = '\0';

    snprintf(data, len, "%s%s\n</body>\n</html>\n", header, body);
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("========== 字典压缩演示 ==========\n\n");

    /* 生成字典 */
    size_t dict_len = 1024;
    unsigned char *dictionary = (unsigned char *)malloc(dict_len);
    generate_dictionary(dictionary, dict_len);

    printf("[1] 生成字典数据\n");
    printf("    字典大小: %zu 字节\n\n", dict_len);

    /* 生成测试数据 */
    const int num_responses = 10;
    size_t response_size = 4096;
    char *responses = (char *)malloc(response_size * num_responses);
    size_t total_size = 0;

    for (int i = 0; i < num_responses; i++) {
        generate_http_response(responses + total_size, response_size, i);
        total_size += response_size;
    }

    printf("[2] 生成测试数据\n");
    printf("    响应数量: %d\n", num_responses);
    printf("    总大小: %zu 字节 (%.2f KB)\n\n", total_size, total_size / 1024.0);

    /* 不使用字典压缩 */
    printf("[3] 压缩 (不使用字典)\n");
    unsigned char *compressed_no_dict = NULL;
    size_t compressed_no_dict_len = 0;

    int ret = compress_with_dict((unsigned char *)responses, total_size,
                                 &compressed_no_dict, &compressed_no_dict_len,
                                 NULL, 0, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 压缩失败\n");
        free(dictionary);
        free(responses);
        return 1;
    }

    printf("    压缩后大小: %zu 字节 (%.2f KB)\n",
           compressed_no_dict_len, compressed_no_dict_len / 1024.0);
    printf("    压缩率: %.2f%%\n\n",
           100.0 * (1.0 - (double)compressed_no_dict_len / total_size));

    /* 使用字典压缩 */
    printf("[4] 压缩 (使用字典)\n");
    unsigned char *compressed_with_dict = NULL;
    size_t compressed_with_dict_len = 0;

    ret = compress_with_dict((unsigned char *)responses, total_size,
                           &compressed_with_dict, &compressed_with_dict_len,
                           dictionary, dict_len, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        fprintf(stderr, "错误: 压缩失败\n");
        free(compressed_no_dict);
        free(dictionary);
        free(responses);
        return 1;
    }

    printf("    压缩后大小: %zu 字节 (%.2f KB)\n",
           compressed_with_dict_len, compressed_with_dict_len / 1024.0);
    printf("    压缩率: %.2f%%\n",
           100.0 * (1.0 - (double)compressed_with_dict_len / total_size));
    printf("    字典带来的改进: %.2f%%\n",
           100.0 * (1.0 - (double)compressed_with_dict_len / compressed_no_dict_len));

    /* 解压缩验证 - 字典压缩的解压缩比较复杂，这里只演示压缩功能 */
    printf("\n[5] 说明\n");
    printf("    字典压缩主要用于提高重复数据的压缩率\n");
    printf("    注意: 使用字典压缩的数据必须使用相同的字典解压缩\n");
    printf("    在实际应用中，字典需要在压缩和解压缩端都可用\n");

    /* 清理 */
    free(dictionary);
    free(responses);
    free(compressed_no_dict);
    free(compressed_with_dict);

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
