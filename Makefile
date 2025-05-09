# === Configuration Flags ===
DEBUG_FLAGS := -D__FINAL__=1
LOG_TYPE    := -D__USE_PRINTF__
BUILD_TYPE  := _final

ifeq ($(DEBUG),1)
    DEBUG_FLAGS := -D__FINAL__=0
    BUILD_TYPE  := _debug
endif

TYPE         := $(BUILD_TYPE)
FINAL        := $(DEBUG_FLAGS)
EXTRAFLAGS   := $(FINAL) $(LOG_TYPE) -fcolor-diagnostics -Wall -D__PRX_BUILD__
# === Libraries ===

LIBRARIES := -lSceLibcInternal -lGoldHEN_Hook -lkernel -lSceSysmodule -lSceNet -lSceSsl -lSceHttp

# === Paths and Names ===
BUILD_FOLDER := $(shell pwd)/../../bin/plugins
OUTPUT_PRX   := $(shell basename $(CURDIR))
TARGET       := $(BUILD_FOLDER)/prx$(TYPE)/$(OUTPUT_PRX)
TARGET_ELF   := $(BUILD_FOLDER)/elf$(TYPE)/$(OUTPUT_PRX)
TARGETSTUB   := $(OUTPUT_PRX).so

TOOLCHAIN    := $(OO_PS4_TOOLCHAIN)
GH_SDK       := $(GOLDHEN_SDK)
EXTERNAL_DIR := ../../external
PROJDIR      := ../$(OUTPUT_PRX)/source
INTDIR       := ../$(OUTPUT_PRX)/build
INCLUDEDIR   := ../$(OUTPUT_PRX)/include
COMMON_DIR   := ../../core

# === Source Files ===
CFILES       := $(wildcard $(PROJDIR)/*.c)
CPPFILES     := $(wildcard $(PROJDIR)/*.cpp)
COMMONFILES  := $(wildcard $(COMMON_DIR)/*.cpp)

OBJS         := $(patsubst $(PROJDIR)/%.c, $(INTDIR)/%.o, $(CFILES)) \
                $(patsubst $(PROJDIR)/%.cpp, $(INTDIR)/%.o, $(CPPFILES)) \
                $(patsubst $(COMMON_DIR)/%.cpp, $(INTDIR)/%.o, $(COMMONFILES))

STUBOBJS     := $(patsubst $(PROJDIR)/%.c, $(INTDIR)/%.o.stub, $(CFILES)) \
                $(patsubst $(PROJDIR)/%.cpp, $(INTDIR)/%.o.stub, $(CPPFILES)) \
                $(patsubst $(COMMON_DIR)/%.cpp, $(INTDIR)/%.o.stub, $(COMMONFILES))

# === Compiler and Linker Configuration ===
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    CC    := clang
    CCX   := clang++
    LD    := ld.lld
    CDIR  := linux
endif
ifeq ($(UNAME_S),Darwin)
    CC    := /usr/local/opt/llvm/bin/clang
    CCX   := /usr/local/opt/llvm/bin/clang++
    LD    := /usr/local/opt/llvm/bin/ld.lld
    CDIR  := macos
endif

CFLAGS   := $(FINAL) --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c \
            $(EXTRAFLAGS) -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include \
            -I$(GH_SDK)/include -I$(INCLUDEDIR) -I$(EXTERNAL_DIR) -I$(COMMON_DIR) $(O_FLAG)

CXXFLAGS := $(CFLAGS) -isystem $(TOOLCHAIN)/$(INCLUDEDIR)/c++/v1

LDFLAGS  := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x -e _init --eh-frame-hdr \
            -L$(TOOLCHAIN)/lib -L$(GH_SDK) $(LIBRARIES) 

# === Build Rules ===

all: plugin_common $(TARGET)

$(TARGET): $(INTDIR) $(OBJS)
	@mkdir -p $(dir $(TARGET_ELF)) $(dir $(TARGET))
	$(LD) $(GH_SDK)/build/crtprx.o $(INTDIR)/*.o -o $(TARGET_ELF).elf $(LDFLAGS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-fself -in=$(TARGET_ELF).elf -out=$(TARGET_ELF).oelf --lib=$(TARGET).prx --paid 0x3800000000000011


$(TARGETSTUB): $(INTDIR) $(STUBOBJS)
	$(CC) $(INTDIR)/*.o.stub -o $(TARGETSTUB) -target x86_64-pc-linux-gnu -shared \
		-fuse-ld=lld -ffreestanding -nostdlib -fno-builtin -L$(TOOLCHAIN)/lib \
		$(LIBRARIES)

$(INTDIR)/%.o: $(PROJDIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(INTDIR)/%.o: $(PROJDIR)/%.cpp
	$(CCX) $(CXXFLAGS) -o $@ $<

$(INTDIR)/%.o.stub: $(PROJDIR)/%.c
	$(CC) -target x86_64-pc-linux-gnu -ffreestanding -nostdlib -fno-builtin -fPIC \
		$(O_FLAG) -s -c -o $@ $<

$(INTDIR)/%.o.stub: $(PROJDIR)/%.cpp
	$(CCX) -target x86_64-pc-linux-gnu -ffreestanding -nostdlib -fno-builtin -fPIC \
		$(O_FLAG) -s -c -o $@ $<

plugin_common:
	@mkdir -p $(INTDIR)
	@find $(COMMON_DIR) -name '*.cpp' | while read file; do \
		$(CCX) $(CXXFLAGS) -c $$file -o $(INTDIR)/$$(basename $$file .cpp).o; \
	done

clean:
	rm -rf $(TARGET) $(TARGETSTUB) $(INTDIR) $(OBJS)

.PHONY: all clean plugin_common
.DEFAULT_GOAL := all
