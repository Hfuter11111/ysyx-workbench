#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Vtop.h"
#include "verilated.h"
#include "verilated_fst_c.h"

int main(int argc, char** argv) {
    uint64_t sim_time = 10;
    VerilatedContext* contextp = new VerilatedContext;      //相当于初始化
    Vtop* top = new Vtop{contextp};                         //使top工作在系统环境中，方便调用仿真的数据（仿真时间等等）
    VerilatedFstC* tfp = new VerilatedFstC;                 //用于生成fst给GTK用,shell命令要加 --trace-fst
    contextp->commandArgs(argc, argv);
    contextp->traceEverOn(true);                            //开轨迹
    top->trace(tfp, 99);                                     //记录电路轨迹
    tfp->open("waveform.fst");                             //将数据写入文件里
    while (contextp->time() < sim_time && !contextp->gotFinish()) {
        int a = rand() & 1;
        int b = rand() & 1;
        top->a = a;
        top->b = b;
        top->eval();
        tfp->dump(contextp->time());
        contextp->timeInc(1);
        printf("a = %d, b = %d, f = %d\n", a, b, top->f);
        assert(top->f == (a ^ b));
    }
    tfp->close();
    delete top;
    delete contextp;
    return 0;
}
