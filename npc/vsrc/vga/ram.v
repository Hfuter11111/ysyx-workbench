module ram #(
  parameter RAM_WIDTH = 32,
  parameter RAM_ADDR_WIDTH = 10
)(
    input clk,
    input we,
    input [RAM_WIDTH-1:0] din,
    input [RAM_ADDR_WIDTH-1:0] inaddr,
    input [RAM_ADDR_WIDTH-1:0] outaddr,
    output [RAM_WIDTH-1:0] dout
);

  reg [RAM_WIDTH-1:0] ram [(2**RAM_ADDR_WIDTH)-1:0];

  initial begin
      $readmemh("source/picture.hex", ram);
  end

  always @(posedge clk)
      if (we)
        ram[inaddr] <= din;

  assign dout = ram[outaddr];

endmodule
