//只考虑顺序按下和松开
module ps2_fsm(key_count, cur_scan, data, scan_valid, clk, clrn);
output reg [7:0] key_count;//按键数
output reg [7:0] cur_scan;//输出当前按键扫描码
input  [7:0] data;
input  clk, scan_valid, clrn;

parameter idle = 2'b00, press = 2'b01, s_break = 2'b10;
reg [1:0] cs, ns;

always @(*) begin
    if (scan_valid) begin
        case(cs)
            idle: ns = press;
            press: ns = (data == 8'hF0) ? s_break : press; //收到F0说明释放否则说明持续按下
            s_break: ns = (data != 8'hF0) ? idle : s_break;
            default: ns = idle;
        endcase
    end
    else 
        ns = cs;
end

always @(posedge clk) begin
    if (!clrn)
        cs <= idle;
    else
        cs <= ns;
end

always @(posedge clk) begin
    if (!clrn) begin
        key_count <= 8'h00;
        cur_scan <= 8'h00;
    end
    else if (scan_valid) begin
        case(cs) 
            idle: begin
                cur_scan <= data;
                key_count <= key_count + 1;
            end
            // 由于即使press时接收到了F0也不一定说明释放了，所以这里press不需
            // 要处理，若收到F0则跳到s_break，交给s_break处理
            s_break: begin
                if (data != 8'hF0) begin
                    cur_scan <= 8'h00; // 确认收到释放键码后再清零
                end
            end
            default: ;
        endcase
    end
end    

endmodule    
