#include "Vbcd7seg.h"
#include <nvboard.h>
#include <nvboard.h>

static Vbcd7seg dut;

void nvboard_bind_all_pins(Vbcd7seg* top);

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

