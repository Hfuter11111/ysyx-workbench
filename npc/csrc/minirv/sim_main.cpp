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

static constexpr uint32_t PMEM_BASE = 0x00000000; 
static constexpr uint32_t PMEM_SIZE = 128 * 1024; // 字节数
static constexpr uint32_t PMEM_WORDS = PMEM_SIZE / 4;

static uint32_t pmem[PMEM_WORDS] {
    0x01400513,
    0x010000e7,
    0x00c000e7,
    0x00100073,
    0x00a50513,
    0x00008067
};

uint32_t pmem_read(uint32_t addr) {

    if(addr < PMEM_BASE || addr > PMEM_BASE + PMEM_SIZE - 4) {
        std::fprintf(stderr, "pmem_read out of range: 0x%08x\n", addr);
        std::exit(EXIT_FAILURE);
    }
    
    const uint32_t offset = (addr - PMEM_BASE) >> 2;
    return pmem[offset];

}

void npc_trap() {
    npcstate = END;
}

void print_registers() {
    for (int i = 0; i < 32; i++) {
        dut->debug_reg_addr = i;
        dut->eval();

        std::printf("x%-2d = 0x%08x%s", i, static_cast<uint32_t>(dut->debug_reg_data), ((i + 1) % 4 == 0) ? "\n" : "    ");
    }
}

int main(int argc, char* argv[]) {
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
        std::printf("pc = 0x%08x, inst = 0x%08x\n", dut->pc, dut->inst);
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
