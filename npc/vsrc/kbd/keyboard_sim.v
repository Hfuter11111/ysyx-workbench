`timescale 1ns / 1ps
module keyboard_sim(output [7:0]h1, h2);

/* parameter */
parameter [31:0] clock_period = 10;

/* ps2_keyboard interface signals */
reg clk,clrn;
wire [7:0] data;
wire ready,overflow;
wire kbd_clk, kbd_data;
reg nextdata_n;
reg [7:0] datatemp;

ps2_keyboard_model model(
    .ps2_clk(kbd_clk),
    .ps2_data(kbd_data)
);

ps2_keyboard inst(
    .clk(clk),
    .clrn(clrn),
    .ps2_clk(kbd_clk),
    .ps2_data(kbd_data),
    .data(data),
    .ready(ready),
    .nextdata_n(nextdata_n),
    .overflow(overflow)
);

bcd7seg bcd1(datatemp[3:0], h1);
bcd7seg bcd2(datatemp[7:4], h2);

initial begin /* clock driver */
    clk = 0;
    forever
        #(clock_period/2) clk = ~clk;
end

initial begin
    clrn = 1'b0;  #20;
    clrn = 1'b1;  #20;
    nextdata_n = 1'b1;
    model.kbd_sendcode(8'h1C); // press 'A'
    #50000
    model.kbd_sendcode(8'hF0); // 释放'A'
    #20000
    model.kbd_sendcode(8'h1c);
    #100000
    $finish;
end

always @(posedge clk) begin
    if (ready) begin
        datatemp <= data;
        nextdata_n = 1'b0;
    end
    else begin
        nextdata_n = 1'b1;
    end
end

endmodule
