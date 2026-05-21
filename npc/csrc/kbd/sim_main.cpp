#include "Vtop.h"
#include <nvboard.h>
#include "verilated.h"

void nvboard_bind_all_pins(Vtop* top);
static Vtop dut; 
void single_cycle() {
  dut.clk = 0; dut.eval();
  dut.clk = 1; dut.eval();
}
int main(int argc, char* argv[]){
    nvboard_bind_all_pins(&dut);
    nvboard_init();

    while (1){
        single_cycle();
        nvboard_update();
        dut.eval();
    }

}

