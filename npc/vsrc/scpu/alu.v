
module alu #(parameter data_width = 8) (
    input [data_width-1:0] a,
    input [data_width-1:0] b,
    input alu_op,
    output [data_width-1:0] result,
    output zero
);
    wire [data_width-1:0] result1;
    wire [data_width-1:0] result2;

    assign result2 = a - b;    
    assign result1 = a + b;

    MuxKey #(2, 1, data_width) 
    mux2_1 (
        .out(result), 
        .key(alu_op), 
        .lut({
            1'b0, result1,
            1'b1, result2
        })
    );

    assign zero = ~(| result);//1有效
endmodule
