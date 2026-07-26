
module IFU #(parameter DATA_WIDTH = 32 ,RESET_PC = 32'h0000_0000) (
    output [DATA_WIDTH-1:0] pc,
    output [DATA_WIDTH-1:0] snpc,
    input [DATA_WIDTH-1:0] dnpc,
    input clk,
    input rst
);

    reg [DATA_WIDTH-1:0] pc_reg;

    assign snpc = pc_reg + 32'd4;
    assign pc = pc_reg;

    always @(posedge clk, posedge rst) begin
        if(rst)
            pc_reg <= RESET_PC;
        else
            pc_reg <= dnpc;
    end

endmodule
