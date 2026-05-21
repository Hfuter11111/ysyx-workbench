#include "Valu.h"
#include "nvboard.h"
#include "verilated.h"

void nvboard_bind_all_pins(Valu*);

static Valu dut;

int main(int argc, char* argv[]){
    nvboard_bind_all_pins(&dut);
    nvboard_init();

    while (1){
        nvboard_update();
        dut.eval();
    }

    nvboard_quit();
}

