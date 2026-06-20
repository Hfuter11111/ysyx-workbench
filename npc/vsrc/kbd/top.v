module top(h1, h2, h3, h4, h5, h6, clk, clrn, kbd_clk, kbd_data, ready, overflow);
output [7:0] h1, h2, h3, h4, h5, h6;
output ready;
output overflow;
input clk, clrn, kbd_clk, kbd_data;

wire [7:0] key_count;
wire [7:0] cur_scan;
wire [7:0] asc;
wire [7:0] data;
wire nextdata_n;

assign nextdata_n = ~ready; // // 本设计中FSM每个clk周期都能处理一个扫描码，所以采用ready高时自动读走的简化写法。

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

ps2_fsm fsm1 (key_count, cur_scan, data, ready, clk, clrn);

ascii ascii1 (asc, cur_scan);

bcd7seg bcd7seg1(cur_scan[3:0], h1);

bcd7seg bcd7seg2(cur_scan[7:4], h2);

bcd7seg bcd7seg3(asc[3:0], h3);

bcd7seg bcd7seg4(asc[7:4], h4);

bcd7seg bcd7seg5(key_count[3:0], h5);

bcd7seg bcd7seg6(key_count[7:4], h6);

endmodule
