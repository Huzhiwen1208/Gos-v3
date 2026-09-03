AsmCompile=nasm
CCompile=gcc
LdCompile=ld
ObjcopyCompile=objcopy
IMG=img/gos.img

GccFlags=-m32 -fno-builtin -fno-stack-protector -march=pentium
GccFlags+=-w -nostdinc -nostdlib -fno-pic -fno-pie -g
LdFlags=-m elf_i386 -static

TARGET=target
BootLoader=src/bootloader
KernelPath=src/kernel

ELFKernel=$(TARGET)/kernel/os.elf
NakedKernel=$(TARGET)/kernel/os.bin
KernelSourceFile=$(wildcard $(KernelPath)/*.c) $(wildcard $(KernelPath)/*.asm)
KernelSourceFile+=$(wildcard $(KernelPath)/*/*.c) $(wildcard $(KernelPath)/*/*.asm)

KernelOBJ=$(patsubst $(KernelPath)/%.asm, $(TARGET)/kernel/%.o, $(filter %.asm, $(KernelSourceFile)))
KernelOBJ+=$(patsubst $(KernelPath)/%.c, $(TARGET)/kernel/%.o, $(filter %.c, $(KernelSourceFile)))
ENTRYPOINT=0x7e00

run: build
	qemu-system-i386 -m 32M \
		-drive file=img/gos.img,if=ide,index=0,media=disk,format=raw

build: $(TARGET) $(IMG)

.PHONY: $(TARGET)
$(TARGET):
ifeq ($(wildcard $(TARGET)),)
	@mkdir -p $(TARGET)/bootloader
	@mkdir -p $(TARGET)/kernel
endif

# .c, .asm ---> .o ------
$(TARGET)/kernel/%.o: $(KernelPath)/%.c
	$(CCompile) $(GccFlags) -c -o $@ $<
$(TARGET)/user/%.o: $(UserPath)/%.c
	$(CCompile) $(GccFlags) -c -o $@ $<
$(TARGET)/kernel/%.o: $(KernelPath)/%.asm
	$(AsmCompile) -f elf32 -g $< -o $@ 
# ----- .c, .asm ---> .o

# bootloader  -------
$(TARGET)/bootloader/%.bin: $(BootLoader)/%.asm
	$(AsmCompile) -o $@ $<
# ------ bootloader

# kernel made ----- 
$(ELFKernel): $(KernelOBJ) $(UserOBJ)
	$(LdCompile) $(LdFlags) -Ttext $(ENTRYPOINT) $^ -o $@
$(NakedKernel): $(ELFKernel)
	$(ObjcopyCompile) -O binary $< $@
# ------ kernel made

# TODO3: 补充 middle.bin 的磁盘写入
# img made --------
$(IMG): $(TARGET)/bootloader/boot.bin $(TARGET)/bootloader/loader.bin \
	$(NakedKernel) image
	dd if=$(word 1, $^) of=$@ bs=512 count=1 conv=notrunc
	dd if=$(word 2, $^) of=$@ bs=512 count=3 seek=1 conv=notrunc
	dd if=$(word 3, $^) of=$@ bs=512 count=250 seek=4 conv=notrunc

image:
ifeq ($(wildcard img),)
	@mkdir img
endif
ifeq ($(wildcard $(IMG)),)
	bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $(IMG)
endif
# ------- img made

debug: build 
	qemu-system-i386 -m 32M \
		-drive file=img/gos.img,if=ide,index=0,media=disk,format=raw \
		-s -S

.PHONY: clean
clean:
	rm -rf *.bin *.o *.lock *.ini *.s *.asm
	rm -rf img
	rm -rf src/target
	rm -rf target

run-mac:
	@make run -f makefile.mac

debug-mac:
	@make debug -f makefile.mac

build-mac:
	@make build -f makefile.mac
