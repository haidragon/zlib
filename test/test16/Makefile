# Makefile for zlib test program - Compression Strategies

# 定义路径
LIBZ_ROOT := $(realpath ../..)
LIBZ_SRC := $(LIBZ_ROOT)
LIBZ_BUILD := $(LIBZ_ROOT)

# 编译器设置
CC := gcc
CFLAGS := -Wall -Wextra -I$(LIBZ_SRC)
LDFLAGS := $(LIBZ_BUILD)/libz.a

# 目标文件
TARGET := test16_compression_strategies
SOURCE := test16_compression_strategies.c
LIBZ_STATIC := $(LIBZ_BUILD)/libz.a
LIBZ_SHARED := $(LIBZ_BUILD)/libz.so
LIBZ_DYLIB := $(LIBZ_BUILD)/libz.dylib

# 默认目标
all: $(TARGET)

# 检查是否需要编译zlib框架
ifeq ($(wildcard $(LIBZ_STATIC)),)
ifeq ($(wildcard $(LIBZ_SHARED)),)
ifeq ($(wildcard $(LIBZ_DYLIB)),)
    NEED_BUILD_ZLIB := yes
endif
endif
endif

# 编译zlib框架（如果需要）
ifeq ($(NEED_BUILD_ZLIB),yes)
    $(info [信息] 未检测到zlib库文件，正在编译zlib框架...)

    build-zlib:
	@echo "[步骤1] 配置zlib..."
	@cd $(LIBZ_BUILD) && ./configure > /dev/null 2>&1 || \
	    (echo "[错误] zlib配置失败"; exit 1)
	@echo "[步骤2] 编译zlib..."
	@$(MAKE) -C $(LIBZ_BUILD) > /dev/null 2>&1 || \
	    (echo "[错误] zlib编译失败"; exit 1)
	@echo "[步骤3] zlib编译完成"
	@ls -lh $(LIBZ_BUILD)/libz.a $(LIBZ_BUILD)/libz.so 2>/dev/null || \
	    ls -lh $(LIBZ_BUILD)/libz.dylib 2>/dev/null || true

    $(TARGET): build-zlib
else
    build-zlib:
	@echo "[信息] zlib库已存在，跳过编译"
	@ls -lh $(LIBZ_BUILD)/libz.a $(LIBZ_BUILD)/libz.so 2>/dev/null || \
	    ls -lh $(LIBZ_BUILD)/libz.dylib 2>/dev/null || true

    $(TARGET): build-zlib
    $(TARGET): $(SOURCE)
endif

# 编译测试程序
$(TARGET): $(SOURCE)
	@echo "[步骤4] 编译测试程序..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)
	@echo "[步骤5] 编译完成"
	@ls -lh $(TARGET)

# 运行测试
test: $(TARGET)
	@echo ""
	@echo "========== 运行测试程序 =========="
	@./$(TARGET)

# 清理
clean:
	@echo "清理测试文件..."
	@rm -f $(TARGET)
	@echo "清理完成"

.PHONY: all build-zlib test clean
