module Led (
    input clk,
    input rst,
    output reg [7:0]led
);

    always @(posedge clk) begin
        if (rst) begin
            led <= 8'b0000_0001;
        end
        else begin
        if (led == 8'b1000_0000)
            led <= 8'b0000_0001;
        else
            led <= led << 1;
        end
    end

endmodule
