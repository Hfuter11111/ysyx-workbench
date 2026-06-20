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
