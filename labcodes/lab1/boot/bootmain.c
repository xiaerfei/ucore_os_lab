#include <defs.h>
#include <x86.h>
#include <elf.h>

/* *********************************************************************
 * This a dirt simple boot loader, whose sole job is to boot
 * an ELF kernel image from the first IDE hard disk.
 *
 * DISK LAYOUT
 *  * This program(bootasm.S and bootmain.c) is the bootloader.
 *    It should be stored in the first sector of the disk.
 *
 *  * The 2nd sector onward holds the kernel image.
 *
 *  * The kernel image must be in ELF format.
 *
 * BOOT UP STEPS
 *  * when the CPU boots it loads the BIOS into memory and executes it
 *
 *  * the BIOS intializes devices, sets of the interrupt routines, and
 *    reads the first sector of the boot device(e.g., hard-drive)
 *    into memory and jumps to it.
 *
 *  * Assuming this boot loader is stored in the first sector of the
 *    hard-drive, this code takes over...
 *
 *  * control starts in bootasm.S -- which sets up protected mode,
 *    and a stack so C code then run, then calls bootmain()
 *
 *  * bootmain() in this file takes over, reads in the kernel and jumps to it.
 * */
// http://xinqiu.me/2016/03/17/ucore/ 参考答案
#define SECTSIZE        512
#define ELFHDR          ((struct elfhdr *)0x10000)      // scratch space

/* waitdisk - wait for disk ready */
static void
waitdisk(void) {
    // status 寄存器 ： 7:BSY 6:DRDY
    // 1100 0000(0xC0)  0100 0000   判断第6位 设备是否就绪
    while ((inb(0x1F7) & 0xC0) != 0x40)
        /* do nothing */;
}

/* readsect - read a single sector at @secno into @dst */
static void
readsect(void *dst, uint32_t secno) {
    // wait for disk to be ready
    waitdisk();

    outb(0x1F2, 1); // 指定读取或者写入的扇区 8位寄存器 0为整个扇区 count = 1
    outb(0x1F3, secno & 0xFF);// LBA 0~7位
    outb(0x1F4, (secno >> 8) & 0xFF); // LBA 8~15位
    outb(0x1F5, (secno >> 16) & 0xFF);// LBA 16~23位
    outb(0x1F6, ((secno >> 24) & 0xF) | 0xE0);// LBA 24~27位
    outb(0x1F7, 0x20);                      // cmd 0x20 - read sectors 读扇区

    // wait for disk to be ready
    waitdisk();

    // read a sector
    // 从端口1F0H中读取数据到dst中, 每次4字节, 读取0x80次
    insl(0x1F0, dst, SECTSIZE / 4);
}

/* *
 * readseg - read @count bytes at @offset from kernel into virtual address @va,
 * might copy more than asked.
 * */
static void
readseg(uintptr_t va, uint32_t count, uint32_t offset) {
    uintptr_t end_va = va + count;

    // round down to sector boundary
    va -= offset % SECTSIZE;

    // translate from bytes to sectors; kernel starts at sector 1
    uint32_t secno = (offset / SECTSIZE) + 1;

    // If this is too slow, we could read lots of sectors at a time.
    // We'd write more to memory than asked, but it doesn't matter --
    // we load in increasing order.
    for (; va < end_va; va += SECTSIZE, secno ++) {
        readsect((void *)va, secno);
    }
}

/* bootmain - the entry of bootloader */
void
bootmain(void) {
    // read the 1st page off disk
    // ELF Header 4096
    readseg((uintptr_t)ELFHDR, SECTSIZE * 8, 0);
// elf文件 http://leenjewel.github.io/blog/2015/05/26/%5B%28xue-xi-xv6%29%5D-jia-zai-bing-yun-xing-nei-he/
    // is this a valid ELF?
    if (ELFHDR->e_magic != ELF_MAGIC) {
        goto bad;
    }

    struct proghdr *ph, *eph;

    // load each program segment (ignores ph flags)
    // uint phoff; 4 字节，表示该文件的“程序头部表”相对于文件的位置，单位是字节
    ph = (struct proghdr *)((uintptr_t)ELFHDR + ELFHDR->e_phoff);
    // ushort phnum; 2 字节，表示程序头部表的入口个数，
    // phnum * phentsize = 程序头部表大小（单位是字节）
    eph = ph + ELFHDR->e_phnum;
    for (; ph < eph; ph ++) {
        // uint memsz;4 字节， 段在内存中的长度
        // uint off;4 字节， 段的第一个字节在文件中的偏移
        readseg(ph->p_va & 0xFFFFFF, ph->p_memsz, ph->p_offset);
    }

    // call the entry point from the ELF header
    // note: does not return
    ((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))();

bad:
    outw(0x8A00, 0x8A00);
    outw(0x8A00, 0x8E00);

    /* do nothing */
    while (1);
}

