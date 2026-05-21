module barrel_shifter #(parameter WIDTH = 8) (
    output [WIDTH-1:0] dout,
    input [WIDTH-1:0] din,
    input  A_L, L_R,
    input [2:0] shmat
);
    
    wire state1;
    MuxKey #(2, 1, 1) mux1 (state1, A_L, {
        1'b0, 1'b0,
        1'b1, din[7]
    });

    wire [WIDTH-1:0] temp1;
    MuxKey #(4, 2, 8) mux2 (temp1, {L_R, shmat[0]}, {
        2'b00, din,
        2'b01, {state1, din[7:1]},
        2'b10, din,
        2'b11, {din[6:0], 1'b0}
    });

    wire [WIDTH-1:0] temp2;
    MuxKey #(4, 2, 8) mux3 (temp2, {L_R, shmat[1]}, {
        2'b00, temp1,
        2'b01, {{2{state1}}, din[7:2]},
        2'b10, temp1,
        2'b11, {din[5:0], 2'b0}
    });

    MuxKey #(4, 2, 8) mux4 (dout, {L_R, shmat[2]}, {
        2'b00, temp2,
        2'b01, {{4{state1}}, din[7:4]},
        2'b10, temp2,
        2'b11, {din[3:0], 4'b0}
    });
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
