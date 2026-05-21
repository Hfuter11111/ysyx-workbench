#include "Vencode83.h"
#include "verilated.h"
#include <nvboard.h>

static Vencode83 dut;

void nvboard_bind_all_pins(Vencode83* top);

int main(int argc, char** argv) {
    nvboard_bind_all_pins(&dut);
    nvboard_init();
    while (1) {
        nvboard_update();
        dut.eval();
        nvboard_update();
    }
    nvboard_quit();
    return 0;
}
