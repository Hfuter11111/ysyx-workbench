#include "Vtop.h"
#include "verilated.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <Vtop__Dpi.h>

static Vtop *dut;

enum NPCState {
    RUNNING,
    END,
    ABORT
};

NPCState npcstate;

static constexpr uint32_t PMEM_BASE = 0x80000000; 
static constexpr uint32_t PMEM_SIZE = 512 * 1024; // 字节数
static constexpr uint32_t PMEM_WORDS = PMEM_SIZE / 4;

static uint32_t pmem[PMEM_WORDS] {};

extern "C" void npc_trap() {
    npcstate = END;
}

extern "C" int pmem_read(int raddr) {
    const uint32_t aligned_addr = static_cast<uint32_t>(raddr) & ~0x3u;

    if(aligned_addr < PMEM_BASE || aligned_addr > PMEM_BASE + PMEM_SIZE - 4) {
        std::fprintf(stderr, "pmem_read out of range: 0x%08x\n", aligned_addr);
        std::exit(EXIT_FAILURE);
    }
    
    const uint32_t offset = (aligned_addr - PMEM_BASE) >> 2;
    return static_cast<int>(pmem[offset]);
}

extern "C" void pmem_write(int waddr, int wdata, char wmask) {
    const uint32_t aligned_addr = static_cast<uint32_t>(waddr) & ~0x3u;
    const uint32_t data = static_cast<uint32_t>(wdata);
    const uint8_t mask = static_cast<uint8_t>(wmask) & 0x0fu;
    
    if(aligned_addr < PMEM_BASE || aligned_addr > PMEM_BASE + PMEM_SIZE - 4) {
        std::fprintf(stderr, "pmem_write out of range: 0x%08x\n", aligned_addr);
        std::exit(EXIT_FAILURE);
    }

    // 总是往地址为`waddr & ~0x3u`的4字节按写掩码`wmask`写入`wdata`
    const uint32_t offset = (aligned_addr - PMEM_BASE) >> 2;
    uint32_t old_data = pmem[offset];
    int i;
    for(i = 0; i < 4; i++) {
        // `wmask`中每比特表示`wdata`中1个字节的掩码,
        if(mask & (1u << i)) {
            old_data = (old_data & ~(0xffu << i*8)) | (data & (0xffu << i*8));
        }
    }
    pmem[offset] = old_data;
}

static void print_registers() {
    for (int i = 0; i < 16; i++) {
        dut->debug_reg_addr = i;
        dut->eval();

        std::printf("x%-2d = 0x%08x%s", i, static_cast<uint32_t>(dut->debug_reg_data), ((i + 1) % 4 == 0) ? "\n" : "    ");
    }
}

static void printf_pc(uint32_t pc, uint32_t inst) {
    std::printf("pc = 0x%08x, inst = 0x%08x\n", pc, inst);
}

static bool load_image(char *img_file) {
    FILE *fp = std::fopen(img_file, "rb");
    if (fp == nullptr) {
        std::perror("Error opening image");
        return false;
    }
    std::memset(pmem, 0, sizeof(pmem));
    const size_t read_bytes = std::fread(pmem, 1, sizeof(pmem), fp);
    if (std::ferror(fp)) {
        std::perror("Error reading image");
        std::fclose(fp);
        return false;
    }
    std::fclose(fp);
    std::printf("Load image: %s, size = %zu bytes\n", img_file, read_bytes);
    return true;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <image.bin>\n", argv[0]);
        exit(1);
    }

    if (!load_image(argv[1])) {
        exit(1);
    }

    dut = new Vtop;
    npcstate = RUNNING;

    dut->clk = 0;
    dut->rst = 1;
    dut->inst = 0;
    dut->eval();
    dut->rst = 0;
    dut->eval();

    while (1){
        dut->clk = 0;
        dut->inst = pmem_read(dut->pc);
        // printf_pc(dut->pc, dut->inst);

        dut->eval();

        dut->clk = 1;
        dut->eval();  

        dut->clk = 0;
        dut->eval();

        if(npcstate == END) {
            print_registers();
            std::printf("HIT GOOD TRAP\n");
            exit(0);
        }
    }
}
