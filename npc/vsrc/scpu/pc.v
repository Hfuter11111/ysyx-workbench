module PC #(parameter pc_width = 4) (
    output [pc_width-1:0] pc,
    input pc_sel,
    input clk,
    input en,
    input rst,
    input [pc_width-1:0] pc_bner
);

    wire [pc_width-1:0] pc_next;
    wire [pc_width-1:0] pc_plus;

    assign pc_plus = pc + 4'h1;
    
    MuxKey #(2, 1, pc_width) 
    mux2_1 (
        .out(pc_next), 
        .key(pc_sel), 
        .lut({
            1'b0, pc_plus,
            1'b1, pc_bner
        })
    );

    Reg #(.WIDTH(pc_width)) Reg_pc(
        .clk(clk),
        .rst(rst),
        .din(pc_next),
        .dout(pc),
        .wen(en)
    );    

endmodule

