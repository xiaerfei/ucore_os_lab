#include <asm.h>
# 解释：http://aprilwith.me/ucore-ex1-notes.html
# Start the CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

# 段描述符的大小为 64 位
# 0x00 ---> 第 0 个选择子
# 0x08 ---> 第 1 个选择子 代码段 PROT_MODE_CSEG
# 0x10 ---> 第 2 个选择子 数据段 PROT_MODE_DSEG

.set PROT_MODE_CSEG,        0x8                     # kernel code segment selector
.set PROT_MODE_DSEG,        0x10                    # kernel data segment selector
.set CR0_PE_ON,             0x1                     # protected mode enable flag

# start address should be 0:7c00, in real mode, the beginning address of the running bootloader
.globl start
start:
.code16                                             # Assemble for 16-bit mode
# Clear interrupt flag; interrupts disabled when interrupt flag cleared.
# CLI中断标志置0指令(Clear interrupt)IF<-0
    cli                                             # Disable interrupts 
# DF：方向标志。DF=0时，每次执行字符串指令后，源或目的地址指针用加法自动地修改地址；
# DF=1时用减法修改地址。它用来控制地址的方向变化。
# CLD方向标志置0指令(Clear direction)DF<-0
# http://blog.sina.com.cn/s/blog_54f82cc2010142zh.html
    cld                                             # String operations increment 

    # Set up the important data segment registers (DS, ES, SS).
    xorw %ax, %ax                                   # Segment number zero
    movw %ax, %ds                                   # -> Data Segment
    movw %ax, %es                                   # -> Extra Segment
    movw %ax, %ss                                   # -> Stack Segment

    # Enable A20:
    #  For backwards compatibility with the earliest PCs, physical
    #  address line 20 is tied low, so that addresses higher than
    #  1MB wrap around to zero by default. This code undoes this.
seta20.1:
# http://blog.csdn.net/bekilledlzy/article/details/1765802
# 0x64为键盘控制电路设备的端口号
    inb $0x64, %al  # Wait for not busy(8042 input buffer empty). 读取端口状态
    testb $0x2, %al # 键盘控制电路第2位为0则表示键盘控制电路空闲   判断键盘是否空闲
    jnz seta20.1    # 第二位是否为零，不为零跳转seta20.1
# 0xd1的意思是向8042端口的P2写数据 
# 向键盘控制器0x64端口号写一个字节，就是发送一个键盘控制器命令，
# 命令可带一个参数，参数从0x60端口写入
    movb $0xd1, %al                                 # 0xd1 -> port 0x64
    outb %al, $0x64                                 # 0xd1 means: write data to 8042's P2 port

seta20.2:
# 判断键盘是否繁忙 
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.2
# P2原IBMPC使用输出端口的位2控制A20门
    movb $0xdf, %al                                 # 0xdf -> port 0x60
    outb %al, $0x60                                 # 0xdf = 11011111, means set P2's A20 bit(the 1 bit) to 1

    # Switch from real to protected mode, using a bootstrap GDT
    # and segment translation that makes virtual addresses
    # identical to physical addresses, so that the
    # effective memory map does not change during the switch.
    lgdt gdtdesc
    movl %cr0, %eax
    orl $CR0_PE_ON, %eax
    movl %eax, %cr0

    # Jump to next instruction, but in 32-bit code segment.
    # Switches processor into 32-bit mode.
    # http://m.newsmth.net/article/Assembler/7547
    ljmp $PROT_MODE_CSEG, $protcseg

.code32                                             # Assemble for 32-bit mode
protcseg:
    # Set up the protected-mode data segment registers
    movw $PROT_MODE_DSEG, %ax                       # Our data segment selector
    movw %ax, %ds                                   # -> DS: Data Segment
    movw %ax, %es                                   # -> ES: Extra Segment
    movw %ax, %fs                                   # -> FS
    movw %ax, %gs                                   # -> GS
    movw %ax, %ss                                   # -> SS: Stack Segment

    # Set up the stack pointer and call into C. The stack region is from 0--start(0x7c00)
    movl $0x0, %ebp
    movl $start, %esp
    call bootmain

    # If bootmain returns (it shouldn't), loop.
spin:
    jmp spin

# Bootstrap GDT
.p2align 2                                          # force 4 byte alignment
gdt:
    SEG_NULLASM                                     # null seg
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)           # code seg for bootloader and kernel
    SEG_ASM(STA_W, 0x0, 0xffffffff)                 # data seg for bootloader and kernel

gdtdesc:
# 为什么是 0x17 ?
# 0x00 ~ 0x07
# 0x08 ~ 0x0f
# 0x10 ~ 0x17
    .word 0x17                                      # sizeof(gdt) - 1
    .long gdt                                       # address gdt
