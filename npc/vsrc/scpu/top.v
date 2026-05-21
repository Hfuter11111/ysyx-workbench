module top(
    output [7:0] h0,
    output [7:0] h1,
    input clk,
    input rst,
    input en
);

    wire [7:0] outdata;

    scpu scpu1(
        .clk(clk),
        .rst(rst),
        .en(en),
        .outdata(outdata)
    );

    bcd7seg seg0(
        .b(outdata[3:0]),
        .h(h0)
    );

    bcd7seg seg1(
        .b(outdata[7:4]),
        .h(h1)
    );

endmodule