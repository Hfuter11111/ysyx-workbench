module bcd7seg(
    input  [3:0] b,
    output [7:0] h
);
    assign h[7] = (b == 4'h1 || b == 4'h4 || b == 4'hb || b == 4'hd);
    assign h[6] = (b == 4'h5 || b == 4'h6 || b == 4'hb || b == 4'hc || b == 4'he || b == 4'hf);
    assign h[5] = (b == 4'h2 || b == 4'hc || b == 4'he || b == 4'hf);
    assign h[4] = (b == 4'h1 || b == 4'h4 || b == 4'h7 || b == 4'ha || b == 4'hf);
    assign h[3] = (b == 4'h1 || b == 4'h3 || b == 4'h4 || b == 4'h5 || b == 4'h7 || b == 4'h9);
    assign h[2] = (b == 4'h1 || b == 4'h2 || b == 4'h3 || b == 4'h7 || b == 4'hd);
    assign h[1] = (b == 4'h0 || b == 4'h1 || b == 4'h7 || b == 4'hc);
    assign h[0] = 1'b1;
endmodule
