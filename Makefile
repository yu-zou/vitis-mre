.PHONY: all

all: vadd.sw_emu.xclbin vadd_test

vadd.sw_emu.xclbin: vadd.cpp
	v++ -t sw_emu --platform xilinx_u200_gen3x16_xdma_2_202110_1 -c -g -k vadd -o vadd.sw_emu.xo vadd.cpp
	v++ -t sw_emu --platform xilinx_u200_gen3x16_xdma_2_202110_1 -l -g \
		--config ./system.cfg \
		-o vadd.sw_emu.xclbin vadd.sw_emu.xo

vadd_test: host.cpp
	g++ -std=c++14 -g -I$(XILINX_XRT)/include -L$(XILINX_XRT)/lib -lxrt_coreutil -pthread -I$(XILINX_HLS)/include -o vadd_test host.cpp

run: vadd_test vadd.sw_emu.xclbin
	XCL_EMULATION_MODE=sw_emu ./vadd_test 0 vadd.sw_emu.xclbin vadd

clean:
	rm -rf *.log *.xclbin* *.xo* _x vadd_test
