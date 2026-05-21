#include "Vencode42.h"
#include "verilated.h"
#include <nvboard.h>

static Vencode42 dut;

void nvboard_bind_all_pins(Vencode42* top);

int main(int argc, char** argv) {
    nvboard_bind_all_pins(&dut);
    nvboard_init();
    while (1) {
        nvboard_update();
        dut.eval();
    }
    nvboard_quit();
    return 0;
}
