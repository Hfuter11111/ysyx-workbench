module shift_register #(parameter WIDTH = 8) ( 
    output [WIDTH-1:0] dout,
    input din,
    input clk, dir, rst
);

    wire [WIDTH-1:0] date;
    Reg #(1, 0) r0 (clk, rst, date[0], dout[0], 1);
    Reg #(1, 0) r1 (clk, rst, date[1], dout[1], 1);
    Reg #(1, 0) r2 (clk, rst, date[2], dout[2], 1);
    Reg #(1, 0) r3 (clk, rst, date[3], dout[3], 1);
    Reg #(1, 0) r4 (clk, rst, date[4], dout[4], 1);
    Reg #(1, 0) r5 (clk, rst, date[5], dout[5], 1);
    Reg #(1, 0) r6 (clk, rst, date[6], dout[6], 1);
    Reg #(1, 0) r7 (clk, rst, date[7], dout[7], 1);

    MuxKey #(2, 1, 8) i0 (date, dir, {
        1'b0, {dout[6:0],din},
        1'b1, {din,dout[7:1]}
    });
endmodule

