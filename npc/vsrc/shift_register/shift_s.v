module shift_register_s #(parameter WIDTH = 8) (
    output [WIDTH-1:0] dout,

    input  [WIDTH-1:0] load_data,   // 置数用的8位并行输入
    input              serial_in,   // 串行输入1位值

    input              clk,
    input              rst,
    input  [2:0]       ctrl         // 3位控制信号
);

    wire [WIDTH-1:0] data;

    Reg #(1, 0) r0 (clk, rst, data[0], dout[0], 1'b1);
    Reg #(1, 0) r1 (clk, rst, data[1], dout[1], 1'b1);
    Reg #(1, 0) r2 (clk, rst, data[2], dout[2], 1'b1);
    Reg #(1, 0) r3 (clk, rst, data[3], dout[3], 1'b1);
    Reg #(1, 0) r4 (clk, rst, data[4], dout[4], 1'b1);
    Reg #(1, 0) r5 (clk, rst, data[5], dout[5], 1'b1);
    Reg #(1, 0) r6 (clk, rst, data[6], dout[6], 1'b1);
    Reg #(1, 0) r7 (clk, rst, data[7], dout[7], 1'b1);

    MuxKey #(8, 3, 8) i0 (data, ctrl, {
        3'b000, 8'b0000_0000,             // 清0
        3'b001, load_data,                // 置数
        3'b010, {1'b0, dout[7:1]},        // 逻辑右移
        3'b011, {dout[6:0], 1'b0},        // 逻辑左移
        3'b100, {dout[7], dout[7:1]},     // 算术右移
        3'b101, {serial_in, dout[7:1]},   // 左端串行输入1位，并行输出8位
        3'b110, {dout[0], dout[7:1]},     // 循环右移
        3'b111, {dout[6:0], dout[7]}      // 循环左移
    });

endmodule
