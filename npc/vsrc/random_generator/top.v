module top(
    output [7:0] h1, h2,
    input clk, rst, dir
);
    wire [7:0] dout;
    shift_register s1(dout, clk, rst);
    bcd7seg b1(dout[3:0], h1);
    bcd7seg b2(dout[7:4], h2);
endmodule

module shift_register #(parameter WIDTH = 8) (
    output [WIDTH-1:0] dout,
    input clk, rst
);
    //复位时置数8‘b00000001
    wire din;
    wire [WIDTH-1:0] date;

    assign din = dout[4] ^ dout[3] ^ dout[2] ^ dout[0];
    assign date = {din,dout[7:1]};

    Reg #(1, 1) r0 (clk, rst, date[0], dout[0], 1);
    Reg #(1, 0) r1 (clk, rst, date[1], dout[1], 1);
    Reg #(1, 0) r2 (clk, rst, date[2], dout[2], 1);
    Reg #(1, 0) r3 (clk, rst, date[3], dout[3], 1);
    Reg #(1, 0) r4 (clk, rst, date[4], dout[4], 1);
    Reg #(1, 0) r5 (clk, rst, date[5], dout[5], 1);
    Reg #(1, 0) r6 (clk, rst, date[6], dout[6], 1);
    Reg #(1, 0) r7 (clk, rst, date[7], dout[7], 1);
    
endmodule

module bcd7seg(
    input  [3:0] b,
    output [7:0] h
);
    assign h[7] = (b == 4'h1 || b == 4'h4 || b == 4'hb || b == 4'hd);
    assign h[6] = (b == 4'h5 || b == 4'h6 || b == 4'hb || b == 4'hc || b == 4'he || b == 4'hf);
    assign h[5] = (b == 4'h2 || b == 4'hc || b == 4'he || b == 4'hf);
    assign h[4] = (b == 4'h1 || b == 4'h4 || b == 4'h7 || b == 4'ha || b == 4'hf);
    assign h[3] = (b == 4'h1 || b == 4'h3 || b == 4'h4 || b == 4'h5 || b == 4'h7 || b == 4'h9);
    assign h[2] = (b == 4'h1 || b == 4'h2 || b == 4'h3 || b == 4'h7 || b == 4'hd);
    assign h[1] = (b == 4'h0 || b == 4'h1 || b == 4'h7 || b == 4'hc);
    assign h[0] = 1'b1;
endmodule

module Reg #(WIDTH = 1, RESET_VAL = 0) (
  input clk,
  input rst,
  input [WIDTH-1:0] din,
  output reg [WIDTH-1:0] dout,
  input wen
);
  always @(posedge clk) begin
    if (rst) dout <= RESET_VAL;
    else if (wen) dout <= din;
  end

endmodule

module MuxKeyInternal #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1, HAS_DEFAULT = 0) (
  output reg [DATA_LEN-1:0] out,
  input [KEY_LEN-1:0] key,
  input [DATA_LEN-1:0] default_out,
  input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
);

  localparam PAIR_LEN = KEY_LEN + DATA_LEN;
  wire [PAIR_LEN-1:0] pair_list [NR_KEY-1:0];
  wire [KEY_LEN-1:0] key_list [NR_KEY-1:0];
  wire [DATA_LEN-1:0] data_list [NR_KEY-1:0];

  genvar n;
  generate
    for (n = 0; n < NR_KEY; n = n + 1) begin
      assign pair_list[n] = lut[PAIR_LEN*(n+1)-1 : PAIR_LEN*n];
      assign data_list[n] = pair_list[n][DATA_LEN-1:0];
      assign key_list[n]  = pair_list[n][PAIR_LEN-1:DATA_LEN];
    end
  endgenerate

  reg [DATA_LEN-1 : 0] lut_out;
  reg hit;
  integer i;
  always @(*) begin
    lut_out = 0;
    hit = 0;
    for (i = 0; i < NR_KEY; i = i + 1) begin
      lut_out = lut_out | ({DATA_LEN{key == key_list[i]}} & data_list[i]);
      hit = hit | (key == key_list[i]);
    end
    if (!HAS_DEFAULT) out = lut_out;
    else out = (hit ? lut_out : default_out);
  end
endmodule

// 不带默认值的选择器模板
module MuxKey #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1) (
  output [DATA_LEN-1:0] out,
  input [KEY_LEN-1:0] key,
  input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
);
  MuxKeyInternal #(NR_KEY, KEY_LEN, DATA_LEN, 0) i0 (out, key, {DATA_LEN{1'b0}}, lut);
endmodule

