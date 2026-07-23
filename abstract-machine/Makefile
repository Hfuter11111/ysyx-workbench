# Makefile for AbstractMachine Kernels and Libraries

### *Get a more readable version of this Makefile* by `make html` (requires python-markdown)
html:
	cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown_py > Makefile.html 
.PHONY: html

## 1. Basic Setup and Checks

## MAKECMDGOALS 保存用户在命令行中指定的目标
## 参数1 = 参数2 则执行
## ifeq (参数1,参数2)
##  条件成立时执行的内容
## endif
## .DEFAULT_GOAL 是 GNU Make 的一个特殊内置变量，用来指定：
## 用户只输入 make、没有写目标时，默认构建哪个目标。
### Default to create a bare-metal kernel image
## 用于确定构建目标
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = image		## 用于后续判断和提示使用
  .DEFAULT_GOAL = image		## 默认构建image make = make image
endif

### Override checks when `make clean/clean-all/html`
## 如果构建目标为clean/clean-all/html，则跳过检查 
## $(findstring 要找的字符串,从这里面找)
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html),)

### Print build info message
## $(info 要打印的内容)
$(info # Building $(NAME)-$(MAKECMDGOALS) [$(ARCH)])

### Check: environment variable `$AM_HOME` looks sane
ifeq ($(wildcard $(AM_HOME)/am/include/am.h),) 
  $(error $$AM_HOME must be an AbstractMachine repo)
endif

### Check: environment variable `$ARCH` must be in the supported list
## 收集支持的架构
## ls得到.../*.mk; notdir删除目录部分得到*.mk; basename去掉扩展名得到*. 
ARCHS = $(basename $(notdir $(shell ls $(AM_HOME)/scripts/*.mk)))
## $(filter 模式列表,文本列表)
ifeq ($(filter $(ARCHS), $(ARCH)), )
  $(error Expected $$ARCH in {$(ARCHS)}, Got "$(ARCH)") 
endif

### Extract instruction set architecture (`ISA`) and platform from `$ARCH`. Example: `ARCH=x86_64-qemu -> ISA=x86_64; PLATFORM=qemu`
## 拆分架构名
ARCH_SPLIT = $(subst -, ,$(ARCH)) ## $(subst 原字符串,新字符串,目标文本)x86_64-qemu -> x86_64 qemu
ISA        = $(word 1,$(ARCH_SPLIT)) ## word 1 表示取第一个单词, ISA = x86_64
PLATFORM   = $(word 2,$(ARCH_SPLIT))

### Check if there is something to build
## 检查有无定义SRCS
ifeq ($(flavor SRCS), undefined)
  $(error Nothing to build)
endif

### Checks end here
endif

## 2. General Compilation Targets

### Create the destination directory (`build/$ARCH`)
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))

### Compilation targets (a binary image or archive)
IMAGE_REL = build/$(NAME)-$(ARCH)
IMAGE     = $(abspath $(IMAGE_REL))
ARCHIVE   = $(WORK_DIR)/build/$(NAME)-$(ARCH).a

### Collect the files to be linked: object files (`.o`) and libraries (`.a`)
## $(addsuffix 后缀,单词列表) 加后缀 $(addprefix 前缀,单词列表) 加前缀
## 源文件对应的目标文件
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
## 依赖库
LIBS     := $(sort $(LIBS) am klib) # lazy evaluation ("=") causes infinite recursions
## 延迟计算将导致无限递归，故要立即计算
## 初始链接内容
LINKAGE   = $(OBJS)  # library archives are added by LIB_TEMPLATE below

## 3. General Compilation Flags

### (Cross) compilers, e.g., mips-linux-gnu-g++
## CROSS_COMPILE 取决于采用的架构
AS        = $(CROSS_COMPILE)gcc				## 编译c
CC        = $(CROSS_COMPILE)gcc				
CXX       = $(CROSS_COMPILE)g++				## 编译c++
LD        = $(CROSS_COMPILE)ld				## 链接目标文件
AR        = $(CROSS_COMPILE)ar				## 制作静态库
OBJDUMP   = $(CROSS_COMPILE)objdump			## 反汇编
OBJCOPY   = $(CROSS_COMPILE)objcopy			## 转为裸二进制
READELF   = $(CROSS_COMPILE)readelf			## 查看ELF信息

### Compilation flags
## 头文件目录
INC_PATH += $(WORK_DIR)/include $(addsuffix /include/, $(addprefix $(AM_HOME)/, $(LIBS)))
INCFLAGS += $(addprefix -I, $(INC_PATH))

ARCH_H := arch/$(ARCH).h
## 裸机编译参数
## 如果ARCH为riscv64-nemu
## 那么有如下宏定义：define __ISA__ riscv64 define __IAS_RISCV64__ 
## define __ARCH__ riscv64-nemu define __ARCH__ RISCV64_NEMU
## define __PLATFORM__ nemu define __PLATFORM_NEMU
CFLAGS   += -O2 -MMD -Wall -Werror $(INCFLAGS) \
            -D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
            -D__ARCH__=$(ARCH) -D__ARCH_$(shell echo $(ARCH) | tr a-z A-Z | tr - _) \
            -D__PLATFORM__=$(PLATFORM) -D__PLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z | tr - _) \
            -DARCH_H=\"$(ARCH_H)\" \
            -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector \
            -Wno-main -U_FORTIFY_SOURCE -fvisibility=hidden
CXXFLAGS +=  $(CFLAGS) -ffreestanding -fno-rtti -fno-exceptions
## 汇编参数
ASFLAGS  += -MMD $(INCFLAGS)
## 链接参数
## -T.../scripts/linker.ld 使用指定链接脚本
LDFLAGS  += -z noexecstack $(addprefix -T, $(LDSCRIPTS))

## 4. Arch-Specific Configurations
## 加载架构配置
### Paste in arch-specific configurations (e.g., from `scripts/x86_64-qemu.mk`)
-include $(AM_HOME)/scripts/$(ARCH).mk

## 5. Compilation Rules

### Rule (compile): a single `.c` -> `.o` (gcc)
## 编译
## $(dir $@) 取得当前目标的目录
## -c：只编译，不链接
$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo + CC $<
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cc` -> `.o` (g++)
$(DST_DIR)/%.o: %.cc
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cpp` -> `.o` (g++)
$(DST_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.S` -> `.o` (gcc, which preprocesses and calls as)
## 汇编
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo + AS $<
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

ifeq ($(MAKECMDGOALS),archive)
### Rule (archive): objects (`*.o`) -> `ARCHIVE.a` (ar)
$(ARCHIVE): $(OBJS)
	@echo + AR "->" $(shell realpath $@ --relative-to .)
	@$(AR) rcs $@ $^
else
# $(1): library name
## 这时只是把一段文字保存在 LIB_TEMPLATE 中，还没有真正创建规则。
## $(1)模板里的第一个参数
## 模板中的 $$ 是为了延迟展开。第一次展开模板时：$$ -> $ ,再由eval再解析一次
define LIB_TEMPLATE =
$$(AM_HOME)/$(1)/build/$(1)-$$(ARCH).a: force
	@$$(MAKE) -s -C $$(AM_HOME)/$(1) archive
LINKAGE += $$(AM_HOME)/$(1)/build/$(1)-$$(ARCH).a
endef

### Rule (recursive make): build a dependent library (am, klib, ...)
## $(foreach 临时变量,列表,表达式)，依次取出列表中的每个单词，执行同一段表达式，再把每次结果拼起来。
$(foreach lib, $(LIBS), $(eval $(call LIB_TEMPLATE,$(lib))))
endif

### Rule (link): objects (`*.o`) and libraries (`*.a`) -> `IMAGE.elf`, the final ELF binary to be packed into image (ld)
$(IMAGE).elf: $(LINKAGE) $(LDSCRIPTS)
	@echo \# Creating image [$(ARCH)]
	@echo + LD "->" $(IMAGE_REL).elf
ifneq ($(filter $(ARCH),native),)
	@$(CXX) -o $@ -Wl,--whole-archive $(LINKAGE) -Wl,-no-whole-archive $(LDFLAGS_CXX)
else
	@$(LD) $(LDFLAGS) -o $@ --start-group $(LINKAGE) --end-group
endif

### Rule (`#include` dependencies): paste in `.d` files generated by gcc on `-MMD`
## 读取头文件依赖
## -include hello.d hello.d由-MMD自动生成，类似于：hello.o: hello.c am.h klib.h arch/riscv.h
-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))

## 6. Miscellaneous

### Build order control
image: image-dep
archive: $(ARCHIVE)
image-dep: $(IMAGE).elf
.PHONY: image image-dep archive run

### Force to rebuild a rule
force:
.PHONY: force

### Clean a single project (remove `build/`)
clean:
	rm -rf Makefile.html $(WORK_DIR)/build/
.PHONY: clean

### Clean all sub-projects within depth 2 (and ignore errors)
CLEAN_ALL = $(dir $(shell find . -mindepth 2 -name Makefile))
clean-all: $(CLEAN_ALL) clean
$(CLEAN_ALL):
	-@$(MAKE) -s -C $@ clean
.PHONY: clean-all $(CLEAN_ALL)
