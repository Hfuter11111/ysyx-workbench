module top #(parameter ADDR_WIDTH = 5, DATA_WIDTH = 32, RESET_PC = 32'h00000000) (
    input clk,
    input rst,
    input [DATA_WIDTH-1:0] inst,
    input  [ADDR_WIDTH-1:0] debug_reg_addr,
    output [DATA_WIDTH-1:0] debug_reg_data,
    output [DATA_WIDTH-1:0] pc
);

    // PC相关
    wire [DATA_WIDTH-1:0] snpc;
    wire [DATA_WIDTH-1:0] dnpc;

    // 寄存器编号
    wire [ADDR_WIDTH-1:0] rd;
    wire [ADDR_WIDTH-1:0] rs1;
    wire [ADDR_WIDTH-1:0] rs2;

    // 立即数
    wire [DATA_WIDTH-1:0] imm_I;
    wire [DATA_WIDTH-1:0] imm_S;
    wire [DATA_WIDTH-1:0] imm_U;

    // 控制信号
    wire [1:0] imm_type;
    wire       jalr;
    wire       src2_imm;
    wire       reg_wb;
    wire [1:0] wb_sel;
    wire       is_ebreak;

    // 寄存器堆数据
    wire [DATA_WIDTH-1:0] rdata1;
    wire [DATA_WIDTH-1:0] rdata2;

    // 执行结果
    wire [DATA_WIDTH-1:0] alu_result;
    wire [DATA_WIDTH-1:0] jalr_target;

    // 写回数据
    wire [DATA_WIDTH-1:0] wb_data;

    IFU #(
        .DATA_WIDTH(DATA_WIDTH /* default 32 */),
        .RESET_PC  (RESET_PC /* default 32'h0000_0000 */)
     ) ifu (
        .pc  (pc),
        .snpc(snpc),
        .dnpc(dnpc),
        .clk (clk),
        .rst (rst)
    );

    IDU #(
        .ADDR_WIDTH(ADDR_WIDTH /* default 5 */),
        .DATA_WIDTH(DATA_WIDTH /* default 32 */)
     ) idu (
        .inst       (inst),
        .rd         (rd),
        .rs1        (rs1),
        .rs2        (rs2),
        .imm_I      (imm_I),
        .imm_S      (imm_S),
        .imm_U      (imm_U),
        .imm_type   (imm_type),
        .jalr       (jalr),
        .src2_imm   (src2_imm),
        .reg_wb     (reg_wb),
        .wb_sel     (wb_sel),
        .is_ebreak  (is_ebreak) 
    );

    import "DPI-C" function void npc_trap(); 
    always @(posedge clk) begin
        if(is_ebreak) npc_trap();
    end

    RegisterFile #(
        .ADDR_WIDTH(ADDR_WIDTH /* default 4 */),
        .DATA_WIDTH(DATA_WIDTH /* default 32 */)
     ) registerFile (
        .clk        (clk),
        .wdata      (wb_data),
        .waddr      (rd),
        .wen        (reg_wb),
        .raddr1     (rs1),
        .raddr2     (rs2),
        .rdata1     (rdata1),
        .rdata2     (rdata2),
        .debug_addr (debug_reg_addr),
        .debug_data (debug_reg_data)
    );

    EXU #(
        .DATA_WIDTH(DATA_WIDTH /* default 32 */)
     ) exu (
        .rdata1     (rdata1),
        .rdata2     (rdata2),
        .imm_I      (imm_I),
        .imm_S      (imm_S),
        .imm_U      (imm_U),
        .imm_type   (imm_type),
        .src2_imm   (src2_imm),
        .alu_result (alu_result),
        .jalr_target(jalr_target)
    );

    WBU #(
        .DATA_WIDTH(DATA_WIDTH /* default 32 */)
     ) wbu (
        .alu_result (alu_result),
        .jalr_target(jalr_target),
        .snpc       (snpc),
        .wb_sel     (wb_sel),
        .jalr       (jalr),
        .wb_data    (wb_data),
        .dnpc       (dnpc)
    );
endmodule
