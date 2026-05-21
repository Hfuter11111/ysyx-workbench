#include "VLed.h"
#include "verilated.h"
#include <nvboard.h>

static VLed* top = new VLed;

void nvboard_bind_all_pins(VLed* top);

void single_cycle() {
  top->clk = 0; top->eval();
  top->clk = 1; top->eval();
}

void reset(int n) {
  top->rst = 1;
  while (n -- > 0) single_cycle();
  top->rst = 0;
}

int main(int argc, char** argv) {
    nvboard_bind_all_pins(top);
    nvboard_init();
    reset(10);
    while (1) {
        single_cycle();
        nvboard_update();
    }
    nvboard_quit();
    return 0;
}

