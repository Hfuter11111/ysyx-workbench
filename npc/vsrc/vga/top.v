module top(clk, reset, hsync, vsync, valid, vga_r, vga_g, vga_b);
output hsync;
output vsync;
output valid;
output [7:0] vga_r;
output [7:0] vga_g;
output [7:0] vga_b;
input clk;
input reset;

wire pclk;
wire [9:0] h_addr;
wire [9:0] v_addr;
wire [23:0] vga_data;
wire [18:0] vga_addr;//640*v_addr为19位的，故位宽为19
wire [18:0] h_addr_ext;
wire [18:0] v_addr_ext;

assign h_addr_ext = {9'b0, h_addr};
assign v_addr_ext = {9'b0, v_addr};
assign vga_addr = v_addr_ext * 19'd640 + h_addr_ext;

//clkgen #(25000000) clkgen1(
//    clk,
//    reset,
//    1'b1,
//    pclk
//    );

assign pclk = clk; //nvboard的example里直接用的pclk = clk
ram #(24,19) ram1(
    pclk,
    1'b0,
    24'h0,
    19'h0,
    vga_addr,
    vga_data
    );


vga_ctrl ctrl1(
    pclk,     //25MHz时钟
    reset,    //置位
    vga_data, //上层模块提供的VGA颜色数据
    h_addr,   //提供给上层模块的当前扫描像素点坐标
    v_addr,
    hsync,    //行同步和列同步信号
    vsync,
    valid,    //消隐信号
    vga_r,    //红绿蓝颜色信号
    vga_g,
    vga_b
    );

endmodule