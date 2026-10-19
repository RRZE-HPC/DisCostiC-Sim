#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Build pisolver-triad-bench (O3/AVX-512) for DisCostiC-demo measured runs.

CC       = mpiicpc
CFLAGS   = -O3 -fno-alias -xCORE-AVX512 -qopt-zmm-usage=high -diag-disable=10441
CXXFLAGS = -std=c++17
BIN_DIR  = bin
TARGET   = $(BIN_DIR)/plain-mpi

.PHONY: all clean

all: $(TARGET)

$(TARGET): main.cpp configParser.hpp helper.hpp timeRankOP.hpp
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(CXXFLAGS) -DUSE_TRIAD -DUSE_PISOLVER -DNB_ISENDIRECV -o $@ main.cpp
	@echo "Built $@"

clean:
	rm -f $(TARGET)
