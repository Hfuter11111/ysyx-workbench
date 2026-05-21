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

