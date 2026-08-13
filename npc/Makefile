TOPNAME ?=
MODULE ?=
NXDC_FILES = constr/$(MODULE)/top.nxdc
INC_PATH ?=

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc --timing \
				-O3 --x-assign fast --x-initial fast --noassert

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	@mkdir -p $(BUILD_DIR)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

VSRCS = $(abspath ./vsrc/$(MODULE)/*.v) 
CSRCS = $(abspath ./csrc/$(MODULE)/sim_main.cpp)
CSRCS += $(SRC_AUTO_BIND)

VSIM_CSRCS = $(abspath ./csrc/$(MODULE)/sim_main.cpp)
VSIM_OBJ_DIR = $(BUILD_DIR)/vsim_obj_dir
VSIM_BIN = $(BUILD_DIR)/$(TOPNAME)_vsim

include $(NVBOARD_HOME)/scripts/nvboard.mk

INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

all: sim

$(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	@rm -rf $(OBJ_DIR)
	@mkdir -p $(BUILD_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

sim: $(BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@$(BIN)

$(VSIM_BIN): $(VSRCS) $(VSIM_CSRCS)
	@rm -rf $(VSIM_OBJ_DIR)
	@mkdir -p $(BUILD_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) \
		$(VSRCS) $(VSIM_CSRCS) \
		--Mdir $(VSIM_OBJ_DIR) \
		--exe \
		-o $(abspath $(VSIM_BIN))

run: $(VSIM_BIN)
	@$(VSIM_BIN) $(IMG)

Lint:
	verilator --lint-only -Wall $(VSRCS)

clean: 
	rm -rf $(BUILD_DIR)

include ../Makefile

