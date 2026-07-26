module RegisterFile #(ADDR_WIDTH = 4, DATA_WIDTH = 32) (
  input clk,
  input [DATA_WIDTH-1:0] wdata,
  input [ADDR_WIDTH-1:0] waddr,
  input wen,
  input [ADDR_WIDTH-1:0] raddr1,
  input [ADDR_WIDTH-1:0] raddr2,
  output [DATA_WIDTH-1:0] rdata1,
  output [DATA_WIDTH-1:0] rdata2,

  // 调试端口
  input  [ADDR_WIDTH-1:0] debug_addr,
  output [DATA_WIDTH-1:0] debug_data
);
  reg [DATA_WIDTH-1:0] rf [2**ADDR_WIDTH-1:0];

  always @(posedge clk) begin
    if (wen) rf[waddr] <= wdata;
    // $display(
    //   "Register write: x%0d <- 0x%08h", waddr, wdata
    // );
  end
  // 写入x0即使非0,但是读出一定为0,满足ISA语义
  assign rdata1 = (raddr1 == 0) ? {DATA_WIDTH{1'b0}} : rf[raddr1];
  assign rdata2 = (raddr2 == 0) ? {DATA_WIDTH{1'b0}} : rf[raddr2];
  
  assign debug_data = (debug_addr == 0) ? {DATA_WIDTH{1'b0}} : rf[debug_addr];
endmodule
