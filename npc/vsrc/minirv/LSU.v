module LSU #(parameter DATA_WIDTH = 32) (
    input [DATA_WIDTH-1:0] waddr,
    input [DATA_WIDTH-1:0] raddr,
    input [DATA_WIDTH-1:0] wdata,
    input valid,
    input wen,
    input [2:0] funct3,
    output reg [DATA_WIDTH-1:0] rdata
);
    reg [7:0] wmask;
    reg [DATA_WIDTH-1:0] rdata_temp;
    reg [DATA_WIDTH-1:0] wdata_mem;

    import "DPI-C" function int pmem_read(input int raddr);
    import "DPI-C" function void pmem_write(
    input int waddr, input int wdata, input byte wmask);

    always @(*) begin
        case(funct3)
            3'b000: begin
                wmask = 8'b0000_0001 << waddr[1:0];
                wdata_mem = {4{wdata[7:0]}};
            end
            3'b010: begin
                wmask = 8'b0000_1111;
                wdata_mem = wdata;
            end
            default: begin
                wmask = 8'b0000_1111;
                wdata_mem = wdata;
            end
        endcase
    end

    always @(*) begin
        if(funct3 == 3'b010) begin
            rdata = rdata_temp;
        end
        else if(funct3 == 3'b100) begin
            case(raddr[1:0])
                2'b00: rdata = {24'b0, rdata_temp[7:0]};
                2'b01: rdata = {24'b0, rdata_temp[15:8]};
                2'b10: rdata = {24'b0, rdata_temp[23:16]};
                2'b11: rdata = {24'b0, rdata_temp[31:24]};
                default: rdata = rdata_temp;
            endcase 
        end
        else begin
            rdata = rdata_temp;
        end
    end
    
    
    always @(*) begin
    if (valid) begin // 有读写请求时
        rdata_temp = pmem_read(raddr);
        if (wen) begin // 有写请求时
            pmem_write(waddr, wdata_mem, wmask);
        end
    end
    else begin
        rdata_temp = 0;
    end
    end

endmodule
