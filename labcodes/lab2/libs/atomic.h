#ifndef __LIBS_ATOMIC_H__
#define __LIBS_ATOMIC_H__

/*
    http://www.cnblogs.com/yangce/archive/2012/04/16/2910093.html
    http://babybandf.blog.163.com/blog/static/6199353201083035144443/
*/

/* Atomic operations that C can't guarantee us. Useful for resource counting etc.. */

static inline void set_bit(int nr, volatile void *addr) __attribute__((always_inline));
static inline void clear_bit(int nr, volatile void *addr) __attribute__((always_inline));
static inline void change_bit(int nr, volatile void *addr) __attribute__((always_inline));
static inline bool test_bit(int nr, volatile void *addr) __attribute__((always_inline));

/* *
 * set_bit - Atomically set a bit in memory
 * @nr:     the bit to set
 * @addr:   the address to start counting from
 *
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 * */
static inline void
set_bit(int nr, volatile void *addr) {
    /// 将 add 的 nr 位置 1
    asm volatile ("btsl %1, %0" :"=m" (*(volatile long *)addr) : "Ir" (nr));
}

/* *
 * clear_bit - Atomically clears a bit in memory
 * @nr:     the bit to clear
 * @addr:   the address to start counting from
 * */
static inline void
clear_bit(int nr, volatile void *addr) {
    /// 将 add 的 nr 位置 0
    asm volatile ("btrl %1, %0" :"=m" (*(volatile long *)addr) : "Ir" (nr));
}

/* *
 * change_bit - Atomically toggle a bit in memory
 * @nr:     the bit to change
 * @addr:   the address to start counting from
 * */
static inline void
change_bit(int nr, volatile void *addr) {
    /// 将 add 的 nr 位置 取反
    asm volatile ("btcl %1, %0" :"=m" (*(volatile long *)addr) : "Ir" (nr));
}

/* *
 * test_bit - Determine whether a bit is set
 * @nr:     the bit to test
 * @addr:   the address to count from
 * */
static inline bool
test_bit(int nr, volatile void *addr) {
    /*
        btl的功能是测试某个数的特定位是零还是１，测试结果放到CF标志位里。
        然后，sbbl %0, %0，也就是说，让oldbit与oldbit的值带位相减，
        即oldbit-oldbit-CF，如果CF标志位是零，也就是刚才位测试的结果是零的话，
        当然最后返回的oldbit也是零了，如果CF标志位是１，那么返回的就不一样了。返回的值为-1
    */
    int oldbit;
    asm volatile ("btl %2, %1; sbbl %0,%0" : "=r" (oldbit) : "m" (*(volatile long *)addr), "Ir" (nr));
    return oldbit != 0;
}

#endif /* !__LIBS_ATOMIC_H__ */

