#include "Vswitch.h"
#include "verilated.h"
#include <nvboard.h>

static Vswitch dut;

void nvboard_bind_all_pins(Vswitch* top);

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
