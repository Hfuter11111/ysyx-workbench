module EXU #(parameter DATA_WIDTH = 32) (
    input [DATA_WIDTH-1:0] rdata1,
    input [DATA_WIDTH-1:0] rdata2,    
    input [DATA_WIDTH-1:0] imm_I,
    input [DATA_WIDTH-1:0] imm_S,
    input [DATA_WIDTH-1:0] imm_U,
    input [1:0] imm_type,
    input src2_imm,
    input src1_0,
    output [DATA_WIDTH-1:0] alu_result,
    output [DATA_WIDTH-1:0] jalr_target  
);
    wire [DATA_WIDTH-1:0] operand_a;
    wire [DATA_WIDTH-1:0] operand_b;
    reg [DATA_WIDTH-1:0] imm;

    parameter I = 2'b00;
    parameter S = 2'b01;
    parameter U = 2'b10;

    // 立即数选择器
    always @(*) begin
        imm = imm_I;
        case(imm_type)
            I:  imm = imm_I;
            S:  imm = imm_S;
            U:  imm = imm_U;
            default: ;
        endcase
    end

    // 操作数选择器
    assign operand_a = src1_0 ? {DATA_WIDTH{1'b0}} : rdata1;
    assign operand_b = src2_imm ? imm : rdata2;

    // 一个alu加法器，一个jalr跳转目标加法器
    assign alu_result = operand_a + operand_b;
    assign jalr_target = (operand_a + operand_b) & 32'hffff_fffe;

endmodule
