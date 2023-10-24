#include <ap_int.h>
//#include <xrt/xrt_bo.h>
//#include <xrt/xrt_kernel.h>
#include "experimental/xrt_ip.h"
#include "xrt/xrt_bo.h"

#define REG_OFFSET_CSR 0x00

#define IP_START (1 << 0)
#define IP_DONE (1 << 1)
#define IP_IDLE (1 << 2)
#define IP_READY (1 << 3)
#define IP_CONTINUE (1 << 4)
#define IP_RESTART (1 << 5)

#define REG_OFFSET_RES 0x58

#define REG_OFFFSET_A 0x10
#define REG_OFFFSET_B 0x1C
#define REG_OFFFSET_C 0x28
#define REG_OFFFSET_D 0x34
#define REG_OFFFSET_E 0x40
#define REG_OFFSET_SIZE 0x4C

const int N = 128;
const uint64_t a_src_addr = 0x00000000;
const uint64_t b_src_addr = 0x00001000;
const uint64_t c_src_addr = 0x00002000;
const uint64_t d_src_addr = 0x00003000;
const uint64_t o_src_addr = 0x00004000;
const uint64_t cmd_src_addr = 0x00005000;
const ap_uint<128> key = 0xdeadbeef;

typedef struct {
    uint64_t param_l;
    uint64_t param_h;
} param_t;

typedef struct {
    uint64_t res_l;
    uint64_t res_h;
} res_t;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Invalid arguments" << std::endl;
        std::cerr << argv[0] << " [device id] "
                  << "[kernel file] [kernel name]" << std::endl;
        return -1;
    }

    int device_id = atoi(argv[1]);
    std::string kernel_file = argv[2];
    std::string kernel_name = argv[3];

    // initialize and load wukong kernel
    std::cout << "Open device" << std::endl;
    auto device = xrt::device(device_id);

    std::cout << "Load xclbin" << std::endl;
    auto uuid = device.load_xclbin(kernel_file);
    auto kernel = xrt::ip(device, uuid, kernel_name);

    // create a 4K aligned buffer
    char* global_mem;
    posix_memalign((void**)&global_mem, 4096,
                   4096 * 5 /*each buffer occupies 4KB*/);

    auto bo_a = xrt::bo(device, global_mem + a_src_addr, 4096, 0);
    auto bo_b = xrt::bo(device, global_mem + b_src_addr, 4096, 0);
    auto bo_c = xrt::bo(device, global_mem + c_src_addr, 4096, 0);
    auto bo_d = xrt::bo(device, global_mem + d_src_addr, 4096, 0);
    auto bo_o = xrt::bo(device, global_mem + o_src_addr, 4096, 0);

    for (int i = 0; i < 16; i++) global_mem[i] = i;
    memcpy(global_mem + b_src_addr, global_mem + a_src_addr, 4096);
    memcpy(global_mem + c_src_addr, global_mem + a_src_addr, 4096);
    memcpy(global_mem + d_src_addr, global_mem + a_src_addr, 4096);

    bo_a.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    bo_b.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    bo_c.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    bo_d.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    // auto run = kernel(bo_a, bo_b, bo_c, bo_d, bo_o, 16);
    // run.wait();
    kernel.write_register(REG_OFFFSET_A, bo_a.address());
    kernel.write_register(REG_OFFFSET_A + 4, bo_a.address() >> 32);
    kernel.write_register(REG_OFFFSET_B, bo_b.address());
    kernel.write_register(REG_OFFFSET_B + 4, bo_b.address() >> 32);
    kernel.write_register(REG_OFFFSET_C, bo_c.address());
    kernel.write_register(REG_OFFFSET_C + 4, bo_c.address() >> 32);
    kernel.write_register(REG_OFFFSET_D, bo_d.address());
    kernel.write_register(REG_OFFFSET_D + 4, bo_d.address() >> 32);
    kernel.write_register(REG_OFFFSET_E, bo_o.address());
    kernel.write_register(REG_OFFFSET_E + 4, bo_o.address() >> 32);
    uint64_t size = 16;
    kernel.write_register(REG_OFFSET_SIZE, size);
    kernel.write_register(REG_OFFSET_SIZE + 4, size >> 32);

    uint32_t axi_ctrl = IP_START;
    kernel.write_register(REG_OFFSET_CSR, axi_ctrl);
    axi_ctrl = 0;
    while ((axi_ctrl & IP_DONE) != IP_DONE) {
        axi_ctrl = kernel.read_register(REG_OFFSET_CSR);
    }

    bo_o.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

    // res.res_l = kernel.read_register(REG_OFFSET_RES + 4);
    // res.res_l = (res.res_l << 32) | kernel.read_register(REG_OFFSET_RES);
    // res.res_h = kernel.read_register(REG_OFFSET_RES + 12);
    // res.res_h = (res.res_h << 32) | kernel.read_register(REG_OFFSET_RES + 8);

    // uint64_t res = kernel.read_register(REG_OFFSET_RES + 4);
    // res = (res << 32) | kernel.read_register(REG_OFFSET_RES);

    // std::cout << res << std::endl;
}
