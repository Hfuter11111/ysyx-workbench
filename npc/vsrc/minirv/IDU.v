module IDU #(parameter ADDR_WIDTH = 5, DATA_WIDTH = 32) (
    input  [DATA_WIDTH-1:0]  inst,
    output [ADDR_WIDTH-1:0]   rd,
    output [ADDR_WIDTH-1:0]   rs1,
    output [ADDR_WIDTH-1:0]   rs2,
    output [DATA_WIDTH-1:0]   imm_I,
    output [DATA_WIDTH-1:0]   imm_S,
    output [DATA_WIDTH-1:0]   imm_U,
    output reg [1:0] imm_type,
    output reg jalr,
    output reg src2_imm,
    output reg reg_wb,
    output reg [1:0] wb_sel,
    output reg is_ebreak
);

    wire [2:0] funct3;
    wire [6:0] opcode;

    // 数据信号
    assign opcode = inst[6:0];
    assign rd = inst[11:7];
    assign funct3 = inst[14:12];
    assign rs1 = inst[19:15];
    assign rs2 = inst[24:20];

    // 立即数生成
    assign imm_I = {{20{inst[31]}}, inst[31:20]};
    assign imm_S = {{20{inst[31]}}, inst[31:25], inst[11:7]};
    assign imm_U = {inst[31:12], {12'b0}};

    parameter I = 2'b00;
    parameter S = 2'b01;
    parameter U = 2'b10;
    parameter opcode_op_imm   = 7'b0010011;
    parameter opcode_jalr = 7'b1100111;
    parameter opcode_ebreak = 7'b1110011;

    parameter wb_alu = 2'b00;
    parameter wb_mem = 2'b01;
    parameter wb_pc4 = 2'b10;

    // 控制信号
    always @(*) begin
        imm_type = I;
        jalr = 1'b0;
        src2_imm = 0;
        reg_wb = 1'b0;
        wb_sel = wb_alu;
        is_ebreak = 1'b0;
        case(opcode)

            opcode_op_imm: begin
                imm_type = I;
                src2_imm = 1;
                reg_wb = 1'b1;
                wb_sel = wb_alu;
            end

            opcode_jalr: begin
                imm_type = I;
                jalr = 1'b1;
                src2_imm = 1;
                reg_wb = 1'b1;
                wb_sel = wb_pc4;
            end

            opcode_ebreak: begin
                is_ebreak = 1'b1;
            end
            
            default: ;
        endcase
    end
endmodule 

