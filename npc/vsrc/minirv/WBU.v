module WBU #(parameter DATA_WIDTH = 32) (
    input [DATA_WIDTH-1:0] alu_result,
    input [DATA_WIDTH-1:0] jalr_target,
    input [DATA_WIDTH-1:0] snpc,
    input [1:0] wb_sel,
    input jalr,
    output reg [DATA_WIDTH-1:0] wb_data,
    output [DATA_WIDTH-1:0] dnpc
);

    parameter wb_alu = 2'b00;
    parameter wb_mem = 2'b01;
    parameter wb_pc4 = 2'b10;

    always @(*) begin
        case(wb_sel) 
            wb_alu: wb_data = alu_result;
            wb_mem: ;
            wb_pc4: wb_data = snpc;
            default: wb_data = {DATA_WIDTH{1'b0}};
        endcase
    end

    assign dnpc = jalr ? jalr_target : snpc;

endmodule
