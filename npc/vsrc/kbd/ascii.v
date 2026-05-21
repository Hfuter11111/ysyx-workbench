`timescale 1ns / 1ps
module ascii(asc, scan);
    output [7:0] asc;
    input [7:0] scan;
    MuxKeyWithDefault #(48, 8, 8) i0 (asc, scan, 8'h00, {
        // 数字键
        8'h16, 8'h31,  // 1
        8'h1E, 8'h32,  // 2
        8'h26, 8'h33,  // 3
        8'h25, 8'h34,  // 4
        8'h2E, 8'h35,  // 5
        8'h36, 8'h36,  // 6
        8'h3D, 8'h37,  // 7
        8'h3E, 8'h38,  // 8
        8'h46, 8'h39,  // 9
        8'h45, 8'h30,  // 0
        // 字母键（小写）
        8'h15, 8'h71,  // q
        8'h1D, 8'h77,  // w
        8'h24, 8'h65,  // e
        8'h2D, 8'h72,  // r
        8'h2C, 8'h74,  // t
        8'h35, 8'h79,  // y
        8'h3C, 8'h75,  // u
        8'h43, 8'h69,  // i
        8'h44, 8'h6F,  // o
        8'h4D, 8'h70,  // p
        8'h1C, 8'h61,  // a
        8'h1B, 8'h73,  // s
        8'h23, 8'h64,  // d
        8'h2B, 8'h66,  // f
        8'h34, 8'h67,  // g
        8'h33, 8'h68,  // h
        8'h3B, 8'h6A,  // j
        8'h42, 8'h6B,  // k
        8'h4B, 8'h6C,  // l
        8'h1A, 8'h7A,  // z
        8'h22, 8'h78,  // x
        8'h21, 8'h63,  // c
        8'h2A, 8'h76,  // v
        8'h32, 8'h62,  // b
        8'h31, 8'h6E,  // n
        8'h3A, 8'h6D,  // m
        // 符号键
        8'h0E, 8'h60,  // `
        8'h4E, 8'h2D,  // -
        8'h55, 8'h3D,  // =
        8'h54, 8'h5B,  // [
        8'h5B, 8'h5D,  // ]
        8'h5D, 8'h5C,  // \
        8'h4C, 8'h3B,  // ;
        8'h52, 8'h27,  // '
        8'h41, 8'h2C,  // ,
        8'h49, 8'h2E,  // .
        8'h4A, 8'h2F,  // /
        8'h29, 8'h20  // SPACE  
    });
endmodule


module MuxKeyWithDefault #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1) (
  output [DATA_LEN-1:0] out,
  input [KEY_LEN-1:0] key,
  input [DATA_LEN-1:0] default_out,
  input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
);
  MuxKeyInternal #(NR_KEY, KEY_LEN, DATA_LEN, 1) i0 (out, key, default_out, lut);
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
