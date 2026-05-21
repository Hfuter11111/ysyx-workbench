module control #(parameter opcode_width = 2) (
    input [opcode_width-1:0] opcode,
    output add_en,
    output out_en,
    output bner0_en,
    output w_en,
    output alu_op
);
    wire li_en;
    assign add_en = (opcode == 2'b00);
    assign out_en = (opcode == 2'b01);
    assign li_en = (opcode == 2'b10);
    assign bner0_en = (opcode == 2'b11);
    assign w_en = (add_en || li_en);
    assign alu_op = (opcode == 2'b11); //只有bner0是做减法，其他做加法，1为减，0为加

endmodule
