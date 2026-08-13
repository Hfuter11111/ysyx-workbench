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
    wire [2:0] funct3;
    wire [1:0] imm_type;
    wire       jalr;
    wire       src2_imm;
    wire       src1_0;
    wire       reg_wb;
    wire [1:0] wb_sel;
    wire       mem_valid;
    wire       mem_wirte;
    wire       is_ebreak;

    // 寄存器堆数据
    wire [DATA_WIDTH-1:0] rdata1;
    wire [DATA_WIDTH-1:0] rdata2;

    // 执行结果
    wire [DATA_WIDTH-1:0] alu_result;
    wire [DATA_WIDTH-1:0] jalr_target;

    // 访存相关
    wire [DATA_WIDTH-1:0] rdata;

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
        .funct3     (funct3),
        .rd         (rd),
        .rs1        (rs1),
        .rs2        (rs2),
        .imm_I      (imm_I),
        .imm_S      (imm_S),
        .imm_U      (imm_U),
        .imm_type   (imm_type),
        .jalr       (jalr),
        .src2_imm   (src2_imm),
        .src1_0     (src1_0),
        .reg_wb     (reg_wb),
        .wb_sel     (wb_sel),
        .mem_valid  (mem_valid),       
        .mem_write  (mem_wirte),           
        .is_ebreak  (is_ebreak) 
    );

    import "DPI-C" function void npc_trap(); 
    always @(posedge clk) begin
        if(is_ebreak) npc_trap();
    end

    RegisterFile #(
        .DATA_WIDTH(DATA_WIDTH /* default 32 */)
     ) registerFile (
        .clk        (clk),
        .wdata      (wb_data),
        .waddr      (rd[3:0]),
        .wen        (reg_wb),
        .raddr1     (rs1[3:0]),
        .raddr2     (rs2[3:0]),
        .rdata1     (rdata1),
        .rdata2     (rdata2),
        .debug_addr (debug_reg_addr[3:0]),
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
        .src1_0     (src1_0),
        .alu_result (alu_result),
        .jalr_target(jalr_target)
    );
  
    LSU #(
        .DATA_WIDTH(DATA_WIDTH /* default 32 */)
     ) lsu (
        .waddr(alu_result),
        .raddr(alu_result),
        .wdata(rdata2),
        .valid(mem_valid),
        .wen  (mem_wirte),
        .funct3(funct3),
        .rdata(rdata)
    );

    WBU #(
        .DATA_WIDTH(DATA_WIDTH /* default 32 */)
     ) wbu (
        .rdata      (rdata),
        .alu_result (alu_result),
        .jalr_target(jalr_target),
        .snpc       (snpc),
        .wb_sel     (wb_sel),
        .jalr       (jalr),
        .funct3     (funct3),
        .wb_data    (wb_data),
        .dnpc       (dnpc)
    );
endmodule
