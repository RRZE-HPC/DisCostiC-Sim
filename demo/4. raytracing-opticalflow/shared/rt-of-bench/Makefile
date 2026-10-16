#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Build measured Ray-Tracing + Optical Flow solver (base binary).

BUILD_DIR = bin
SRC_DIR = src
TARGET = $(BUILD_DIR)/ray-tracing-optical-flow-base

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(SRC_DIR)/main.cpp $(SRC_DIR)/ray-tracing.h $(SRC_DIR)/ray-tracing-util.h \
           $(SRC_DIR)/intersect.h $(SRC_DIR)/image.h $(SRC_DIR)/vec3.h \
           $(SRC_DIR)/optical-flow.h $(SRC_DIR)/timer.h $(SRC_DIR)/json.hpp
	mkdir -p $(BUILD_DIR)
	mpiicpc -O3 -xCORE-AVX512 -qopt-zmm-usage=high -fno-alias -std=c++17 -g \
	  -o $@ $(SRC_DIR)/main.cpp

clean:
	rm -f $(TARGET)
