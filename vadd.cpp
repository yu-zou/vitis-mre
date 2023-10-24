#include <ap_int.h>
#include <hls_stream.h>
#include <stdint.h>
#ifndef __SYNTHESIS__
#include <cassert>
#include <iomanip>
#include <iostream>
#endif

typedef struct {
    uint64_t res_l;
    uint64_t res_h;
} res_t;

typedef struct {
    uint64_t param_l;
    uint64_t param_h;
} param_t;

void read(uint64_t* mem, hls::stream<ap_uint<512>>& strm, uint64_t n) {
    ap_uint<512> tmp;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 8; j++) {
#pragma HLS pipeline
            tmp(j * 64 + 63, j * 64) = *mem++;
        }
        strm.write(tmp);
    }
}

void add(hls::stream<ap_uint<512>>& strm_a, hls::stream<ap_uint<512>>& strm_b,
         hls::stream<ap_uint<512>>& strm_c, hls::stream<ap_uint<512>>& strm_d,
         hls::stream<ap_uint<512>>& strm_o, uint64_t n) {
    for (int i = 0; i < n; i++) {
#pragma HLS pipeline
        strm_o.write(strm_a.read() + strm_b.read() + strm_c.read() +
                     strm_d.read());
    }
}

void write(uint64_t* mem, hls::stream<ap_uint<512>>& strm, uint64_t n) {
    ap_uint<512> tmp;
    for (int i = 0; i < n; i++) {
        tmp = strm.read();
        for (int j = 0; j < 8; j++) {
#pragma HLS pipeline
            *mem++ = tmp(j * 64 + 63, j * 64);
        }
    }
}

extern "C" {
void vadd(uint64_t* a, uint64_t* b, uint64_t* c, uint64_t* d, uint64_t* e,
          uint64_t size) {
    // clang-format off
#pragma HLS interface mode=m_axi bundle=mem_bus_a port=a
#pragma HLS interface mode=m_axi bundle=mem_bus_b port=b
#pragma HLS interface mode=m_axi bundle=mem_bus_c port=c
#pragma HLS interface mode=m_axi bundle=mem_bus_d port=d
#pragma HLS interface mode=m_axi bundle=mem_bus_e port=e
#pragma HLS interface mode=s_axilite port=size
    // clang-format on
#pragma HLS dataflow
#ifndef __SYNTHESIS__
    std::cout << "size" << std::endl;
    std::cout << std::hex << std::setw(16) << std::setfill('0') << size
              << std::endl;
    std::cout << "a" << std::endl;
    for (int i = 0; i < 4; i++)
        std::cout << std::hex << std::setw(16) << std::setfill('0') << a[i]
                  << std::endl;
    std::cout << "b" << std::endl;
    for (int i = 0; i < 4; i++)
        std::cout << std::hex << std::setw(16) << std::setfill('0') << b[i]
                  << std::endl;
    std::cout << "c" << std::endl;
    for (int i = 0; i < 4; i++)
        std::cout << std::hex << std::setw(16) << std::setfill('0') << c[i]
                  << std::endl;
    std::cout << "d" << std::endl;
    for (int i = 0; i < 4; i++)
        std::cout << std::hex << std::setw(16) << std::setfill('0') << d[i]
                  << std::endl;
#endif
    hls::stream<ap_uint<512>> strm_a;
    hls::stream<ap_uint<512>> strm_b;
    hls::stream<ap_uint<512>> strm_c;
    hls::stream<ap_uint<512>> strm_d;
    hls::stream<ap_uint<512>> strm_o;

    read(a, strm_a, size);
    read(b, strm_b, size);
    read(c, strm_c, size);
    read(d, strm_d, size);
    add(strm_a, strm_b, strm_c, strm_d, strm_o, size);
    write(e, strm_o, size);
}
}
