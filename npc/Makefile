all:
	@echo "Write this Makefile by your self."

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	verilator --cc --exe --build -j 0 -Wall --trace-fst top.v sim_main.cpp
	./obj_dir/Vtop

clean: 
	rm -rf obj_dir
include ../Makefile
