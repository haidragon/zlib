/*
 * test18: 网络数据压缩演示
 *
 * 本示例演示如何压缩网络传输的数据
 * 模拟简单的客户端-服务器通信场景
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK_SIZE 16384

/* 模拟网络数据包 */
typedef struct {
    uLong packet_id;
    uLong data_size;
    unsigned char data[CHUNK_SIZE];
} NetworkPacket;

/* 压缩网络数据包 */
int compress_packet(const NetworkPacket *packet,
                   unsigned char **compressed_data, size_t *compressed_len) {
    uLongf bound = compressBound(packet->data_size);
    unsigned char *compressed = (unsigned char *)malloc(bound);
    if (!compressed) return Z_MEM_ERROR;

    int ret = compress2(compressed, compressed_len,
                      packet->data, packet->data_size, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        free(compressed);
        return ret;
    }

    *compressed_data = compressed;
    return Z_OK;
}

/* 解压缩网络数据包 */
int decompress_packet(const unsigned char *compressed_data, size_t compressed_len,
                     NetworkPacket *packet) {
    uLongf data_size = sizeof(packet->data);
    int ret = uncompress(packet->data, &data_size, compressed_data, compressed_len);
    if (ret != Z_OK) return ret;

    packet->data_size = data_size;
    return Z_OK;
}

/* 模拟创建HTTP请求数据 */
void create_http_request_data(NetworkPacket *packet, uLong packet_id) {
    packet->packet_id = packet_id;

    const char *request = "GET /api/v1/resource HTTP/1.1\r\n"
                         "Host: example.com\r\n"
                         "User-Agent: MyApp/1.0\r\n"
                         "Accept: application/json\r\n"
                         "Connection: keep-alive\r\n"
                         "\r\n";

    size_t len = strlen(request);
    size_t remaining = sizeof(packet->data);

    /* 填充请求数据 */
    memcpy(packet->data, request, len);
    packet->data_size = len;

    /* 添加一些重复内容模拟真实请求 */
    if (remaining > len) {
        memset(packet->data + len, ' ', remaining - len);
        packet->data_size = remaining;
    }
}

/* 模拟创建HTTP响应数据 */
void create_http_response_data(NetworkPacket *packet, uLong packet_id) {
    packet->packet_id = packet_id;

    const char *response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: application/json\r\n"
                          "Content-Length: 8192\r\n"
                          "Connection: keep-alive\r\n"
                          "\r\n"
                          "{\"status\":\"success\",\"data\":[";
    size_t len = strlen(response);
    memcpy(packet->data, response, len);
    size_t pos = len;

    /* 添加JSON数组元素 */
    for (int i = 0; i < 100 && pos < sizeof(packet->data) - 20; i++) {
        pos += snprintf((char *)packet->data + pos, sizeof(packet->data) - pos,
                       "\"item%d\",", i);
    }
    if (pos > 0 && packet->data[pos-1] == ',') pos--;

    pos += snprintf((char *)packet->data + pos, sizeof(packet->data) - pos, "]}");

    packet->data_size = pos;
}

/* 显示数据包信息 */
void print_packet_info(const char *label, const NetworkPacket *packet) {
    printf("  %s:\n", label);
    printf("    包ID: %lu\n", packet->packet_id);
    printf("    数据大小: %lu 字节\n", packet->data_size);
    printf("    数据预览: %.50s...\n", (char *)packet->data);
}

int main(void) {
    printf("========== 网络数据压缩演示 ==========\n\n");

    /* 测试1: HTTP请求压缩 */
    printf("[测试1] HTTP请求数据压缩\n");

    NetworkPacket request_packet;
    create_http_request_data(&request_packet, 1);
    print_packet_info("原始请求", &request_packet);

    unsigned char *compressed = NULL;
    size_t compressed_len = 0;
    int ret = compress_packet(&request_packet, &compressed, &compressed_len);

    if (ret == Z_OK) {
        printf("  压缩后大小: %zu 字节\n", compressed_len);
        printf("  压缩率: %.2f%%\n",
               100.0 * (1.0 - (double)compressed_len / request_packet.data_size));

        /* 解压缩验证 */
        NetworkPacket decompressed_packet;
        ret = decompress_packet(compressed, compressed_len, &decompressed_packet);
        if (ret == Z_OK) {
            printf("  解压缩包ID: %lu\n", decompressed_packet.packet_id);
            printf("  数据一致性: %s\n",
                   memcmp(request_packet.data, decompressed_packet.data,
                         request_packet.data_size) == 0 ? "✓ 通过" : "✗ 失败");
        }
        free(compressed);
    }
    printf("\n");

    /* 测试2: HTTP响应压缩 */
    printf("[测试2] HTTP响应数据压缩\n");

    NetworkPacket response_packet;
    create_http_response_data(&response_packet, 2);
    print_packet_info("原始响应", &response_packet);

    compressed = NULL;
    compressed_len = 0;
    ret = compress_packet(&response_packet, &compressed, &compressed_len);

    if (ret == Z_OK) {
        printf("  压缩后大小: %zu 字节\n", compressed_len);
        printf("  压缩率: %.2f%%\n",
               100.0 * (1.0 - (double)compressed_len / response_packet.data_size));
        free(compressed);
    }
    printf("\n");

    /* 测试3: 多个数据包压缩率对比 */
    printf("[测试3] 多个数据包压缩率对比\n");
    printf("%-10s %-15s %-15s %-10s\n",
           "包ID", "原始大小", "压缩后大小", "压缩率");
    printf("%-10s %-15s %-15s %-10s\n",
           "--------", "---------------", "---------------", "----------");

    for (uLong i = 1; i <= 5; i++) {
        NetworkPacket packet;
        if (i % 2 == 0) {
            create_http_response_data(&packet, i);
        } else {
            create_http_request_data(&packet, i);
        }

        compressed = NULL;
        compressed_len = 0;
        compress_packet(&packet, &compressed, &compressed_len);

        printf("%-10lu %-15lu %-15zu %-10.2f%%\n",
               i, packet.data_size, compressed_len,
               100.0 * (1.0 - (double)compressed_len / packet.data_size));

        free(compressed);
    }

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
