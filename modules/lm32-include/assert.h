#ifndef __ASSERT_H__
#define __ASSERT_H__

/*
 * Both panic and assert loop over the console, re-printing the
 * message, so you can connnect to a paniced node and see the message
 * that caused the panic.  assert_warn(condition) is once only, like
 * warn_on(!condition) in the kernel.
 */

extern void panic(const char *fmt, ...)
	__attribute__((format(printf,1,2)));

#define assert(cond, fmt, ...) \
	if (CONFIG_HAS_ASSERT && !(cond))				\
		__assert(__func__, __LINE__, 1 /* forever */, fmt, ##  __VA_ARGS__)

#define assert_warn(cond, fmt, ...) \
	if (CONFIG_HAS_ASSERT && !(cond))				\
		__assert(__func__, __LINE__, 0 /* once */, fmt, ##  __VA_ARGS__)


extern void __assert(const char *func, int line, int forever,
		     const char *fmt, ...)
	__attribute__((format(printf, 4, 5)));

#ifdef CONFIG_ASSERT
#  define CONFIG_HAS_ASSERT 1
#else
#  define CONFIG_HAS_ASSERT 0
#endif

#endif /* __ASSERT_H__ */
