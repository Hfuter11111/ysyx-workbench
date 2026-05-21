module alu(Result, Carry, Overflow, Zero, A, B, S);
parameter n = 4;
output [n-1:0] Result;
output Carry, Overflow, Zero;
input [n-1:0] A, B;
input [2:0] S;

    //A+B
    wire add_Carry, add_Zero, add_Overflow;
    wire [n-1:0] Result0;
    add_sub a1 (add_Carry, add_Zero, add_Overflow, Result0, 0, A, B);

    //A-B
    wire sub_Carry, sub_Zero, sub_Overflow;
    wire [n-1:0] Result1;
    add_sub a2 (sub_Carry, sub_Zero, sub_Overflow, Result1, 1, A, B);

    //Not A
    wire [n-1:0] Result2;
    assign Result2 = ~A;

    //A and B
    wire [n-1:0] Result3;
    assign Result3 = A & B;

    //A or B
    wire [n-1:0] Result4;
    assign Result4 = A | B;

    //A xor B
    wire [n-1:0] Result5;
    assign Result5 = A ^ B;

    //If A<B then out=1; else out=0;
    wire [n-1:0] Result6;
    assign Result6 = {{n-1{1'b0}}, Result1[n-1] ^ sub_Overflow};

    //If A==B then out=1; else out=0;
    wire [n-1:0] Result7;
    assign Result7 = {{n-1{1'b0}}, sub_Zero};

    MuxKey #(2, 1, 1) i0(Carry, S[0], {
        1'b0, add_Carry,
        1'b1, sub_Carry
    });

    MuxKey #(2, 1, 1) i1(Overflow, S[0], {
        1'b0, add_Overflow,
        1'b1, sub_Overflow
    });

    MuxKey #(2, 1, 1) i2(Zero, S[0], {
        1'b0, add_Zero,
        1'b1, sub_Zero
    });

    MuxKey #(8, 3, 4) i3(Result, S,{
        3'b000, Result0,
        3'b001, Result1,
        3'b010, Result2,
        3'b011, Result3,
        3'b100, Result4,
        3'b101, Result5,
        3'b110, Result6,
        3'b111, Result7
    });

endmodule

module add_sub(Carry, Zero, Overflow, Result, Cin, A, B);
    parameter n = 4;
    output Carry, Zero, Overflow;
    output [n-1:0] Result;
    input Cin;
    input [n-1:0] A, B;
    wire [n-1:0] t_no_Cin;

    assign t_no_Cin = {n{ Cin }}^B;
    assign {Carry,Result} = A + t_no_Cin + Cin;
    assign Overflow = (A[n-1] == t_no_Cin[n-1]) && (Result [n-1] != A[n-1]);
    assign Zero = ~(| Result);
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
