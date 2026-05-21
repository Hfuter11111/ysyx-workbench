
module regfile #(parameter addr_width = 2, data_width = 8) (
    input clk,
    input rst,
    input wen,                    
    input [addr_width-1:0] raddr1,         
    input [addr_width-1:0] raddr2,          
    input [addr_width-1:0] waddr,           
    input [data_width-1:0] wdata,          
    output [data_width-1:0] rdata1,        
    output [data_width-1:0] rdata2      
);

    wire wen0;
    wire wen1;
    wire wen2;
    wire wen3;
    wire [7:0]reg0;
    wire [7:0]reg1;
    wire [7:0]reg2;
    wire [7:0]reg3;

    assign wen0 = wen && (waddr == 2'b00);
    assign wen1 = wen && (waddr == 2'b01);
    assign wen2 = wen && (waddr == 2'b10);
    assign wen3 = wen && (waddr == 2'b11);  

    Reg #(.WIDTH(data_width)) Reg0(
        .clk(clk),
        .rst(rst),
        .din(wdata),
        .dout(reg0),
        .wen(wen0)
    ); 
    Reg #(.WIDTH(data_width)) Reg1(
        .clk(clk),
        .rst(rst),
        .din(wdata),
        .dout(reg1),
        .wen(wen1)
    ); 
    Reg #(.WIDTH(data_width)) Reg2(
        .clk(clk),
        .rst(rst),
        .din(wdata),
        .dout(reg2),
        .wen(wen2)
    ); 
    Reg #(.WIDTH(data_width)) Reg3(
        .clk(clk),
        .rst(rst),
        .din(wdata),
        .dout(reg3),
        .wen(wen3)
    );       

    MuxKey #(4, 2, data_width) 
    mux4_1_1 (
        .out(rdata1), 
        .key(raddr1), 
        .lut({
            2'b00, reg0,
            2'b01, reg1,
            2'b10, reg2,
            2'b11, reg3
        })
    );  

    MuxKey #(4, 2, data_width) 
    mux4_1_2 (
        .out(rdata2), 
        .key(raddr2), 
        .lut({
            2'b00, reg0,
            2'b01, reg1,
            2'b10, reg2,
            2'b11, reg3
        })
    );   
endmodule
