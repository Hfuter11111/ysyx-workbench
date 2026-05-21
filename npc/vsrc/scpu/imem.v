module imem #(parameter pc_width = 4, inst_width = 8) (
    input   [pc_width-1:0] addr,
    output [inst_width-1:0] inst
);

    MuxKeyWithDefault #(.NR_KEY(16), .KEY_LEN(pc_width), .DATA_LEN(inst_width)) 
    mux16_1(
        .out(inst),
        .key(addr),
        .default_out(8'h0),
        .lut({
            4'h00, 8'h8a,
            4'h01, 8'h90,
            4'h02, 8'ha0,
            4'h03, 8'hb1,
            4'h04, 8'h17,
            4'h05, 8'h29,
            4'h06, 8'hd1,
            4'h07, 8'h42,
            4'h08, 8'he3,
            4'h09, 8'h02,
            4'h0a, 8'h03,
            4'h0b, 8'h00,
            4'h0c, 8'h00,
            4'h0d, 8'h00,
            4'h0e, 8'h00,
            4'h0f, 8'h00
        })
    );
endmodule
