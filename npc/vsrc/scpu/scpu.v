module scpu(
    input clk,
    input rst,
    input en,
    output [7:0] outdata
);

    wire [3:0] pc;
    wire pc_sel;
    wire [7:0] inst;
    wire [1:0] opcode;
    wire [1:0] rd;
    wire [1:0] rs1;
    wire [1:0] rs2;
    wire [3:0] imm;
    wire [3:0] pc_bner;
    wire add_en;
    wire out_en;
    wire bner0_en;
    wire w_en;
    wire alu_op;
    wire [1:0] raddr1;
    wire [1:0] raddr2;
    wire [1:0] waddr;
    wire [7:0] wdata;
    wire [7:0] rdata1;
    wire [7:0] rdata2;
    wire [7:0] result;
    wire zero;

    PC PC1(
        .pc(pc),
        .pc_sel(pc_sel),
        .clk(clk),
        .en(en),
        .rst(rst),
        .pc_bner(pc_bner)
    );

    imem imem1(
    .addr(pc),
    .inst(inst)
    ); 

    decoder decoder1(
    .inst(inst),
    .opcode(opcode),
    .rd(rd),
    .rs1(rs1),
    .rs2(rs2),
    .imm(imm),
    .pc_addr(pc_bner)
    );   

    control control1(
    .opcode(opcode),
    .add_en(add_en),
    .out_en(out_en),
    .bner0_en(bner0_en),
    .w_en(w_en),
    .alu_op(alu_op)
    );

    MuxKey #(2, 1, 2)
    mux2_1_1 (
        .out(raddr1), 
        .key(add_en), 
        .lut({
            1'b0, 2'b0,
            1'b1, rs1
        })
    );
    assign raddr2 = rs2;
    assign waddr = rd;

    regfile regfile1(
    .clk(clk),
    .rst(rst),
    .wen(w_en),                    
    .raddr1(raddr1),         
    .raddr2(raddr2),          
    .waddr(waddr),           
    .wdata(wdata),          
    .rdata1(rdata1),        
    .rdata2(rdata2)     
    );

    alu alu1(
    .a(rdata1),
    .b(rdata2),
    .alu_op(alu_op),
    .result(result),
    .zero(zero)
    );

    assign pc_sel = ((bner0_en) && (!zero));
    MuxKey #(2, 1, 8) 
    mux2_1_2 (
        .out(wdata), 
        .key(add_en), 
        .lut({
            1'b0, {4'h0, imm},
            1'b1, result
        })
    );    

    Reg #(.WIDTH(8)) Reg_out(
        .clk(clk),
        .rst(rst),
        .din(rdata2),
        .dout(outdata),
        .wen(out_en)
    );

    
    
endmodule
