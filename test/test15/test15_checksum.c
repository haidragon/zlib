/*
 * test15: 校验和计算演示
 *
 * 本示例演示zlib提供的两种校验和算法:
 * - Adler32: 快速但强度较低的校验和
 * - CRC32: 标准的CRC-32校验和
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zlib.h>

void print_checksum(const char *label, uLong crc) {
    printf("  %s: 0x%08lx (%lu)\n", label, crc, crc);
}

void test_basic_checksums(void) {
    printf("\n[测试1] 基本校验和计算\n");
    const char *test_data = "Hello, World!";
    printf("  测试数据: \"%s\"\n", test_data);

    uLong adler = adler32(0, Z_NULL, 0);
    adler = adler32(adler, (const Bytef *)test_data, strlen(test_data));
    print_checksum("Adler32", adler);

    uLong crc = crc32(0, Z_NULL, 0);
    crc = crc32(crc, (const Bytef *)test_data, strlen(test_data));
    print_checksum("CRC32", crc);
}

void test_incremental_checksums(void) {
    printf("\n[测试2] 增量校验和计算\n");
    const char *part1 = "Hello, ";
    const char *part2 = "World!";

    printf("  分段1: \"%s\"\n", part1);
    printf("  分段2: \"%s\"\n", part2);

    uLong adler = adler32(0, Z_NULL, 0);
    adler = adler32(adler, (const Bytef *)part1, strlen(part1));
    adler = adler32(adler, (const Bytef *)part2, strlen(part2));
    print_checksum("Adler32 (增量)", adler);

    uLong crc = crc32(0, Z_NULL, 0);
    crc = crc32(crc, (const Bytef *)part1, strlen(part1));
    crc = crc32(crc, (const Bytef *)part2, strlen(part2));
    print_checksum("CRC32 (增量)", crc);

    char combined[100];
    strcpy(combined, part1);
    strcat(combined, part2);

    uLong adler_single = adler32(0, Z_NULL, 0);
    adler_single = adler32(adler_single, (const Bytef *)combined, strlen(combined));
    uLong crc_single = crc32(0, Z_NULL, 0);
    crc_single = crc32(crc_single, (const Bytef *)combined, strlen(combined));

    print_checksum("Adler32 (一次性)", adler_single);
    print_checksum("CRC32 (一次性)", crc_single);

    printf("  验证: %s\n",
           (adler == adler_single && crc == crc_single) ? "✓ 一致" : "✗ 不一致");
}

void test_combine_checksums(void) {
    printf("\n[测试3] 组合校验和\n");

    const char *data1 = "First block of data. ";
    const char *data2 = "Second block of data.";
    size_t len1 = strlen(data1);
    size_t len2 = strlen(data2);

    uLong adler1 = adler32(0, Z_NULL, 0);
    adler1 = adler32(adler1, (const Bytef *)data1, len1);

    uLong adler2 = adler32(0, Z_NULL, 0);
    adler2 = adler32(adler2, (const Bytef *)data2, len2);

    printf("  块1校验和: 0x%08lx\n", adler1);
    printf("  块2校验和: 0x%08lx\n", adler2);

    uLong combined_adler = adler32_combine(adler1, adler2, len2);
    print_checksum("组合后的Adler32", combined_adler);

    char combined[256];
    strcpy(combined, data1);
    strcat(combined, data2);
    uLong full_adler = adler32(0, Z_NULL, 0);
    full_adler = adler32(full_adler, (const Bytef *)combined, strlen(combined));

    printf("  整体校验和: 0x%08lx\n", full_adler);
    printf("  验证: %s\n", (combined_adler == full_adler) ? "✓ 一致" : "✗ 不一致");
}

int main(void) {
    printf("========== 校验和计算演示 ==========\n");

    test_basic_checksums();
    test_incremental_checksums();
    test_combine_checksums();

    printf("\n========== 测试完成 ==========\n");
    printf("zlib版本: %s\n", zlibVersion());
    return 0;
}
