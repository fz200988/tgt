#ifndef __UTIL_H__
#define __UTIL_H__

#include <byteswap.h>
#include <syscall.h>
#include <unistd.h>
#include <errno.h>
#include <endian.h>
#include "be_byteshift.h"

#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define __cpu_to_be16(x) bswap_16(x)
#define __cpu_to_be32(x) bswap_32(x)
#define __cpu_to_be64(x) bswap_64(x)
#define __be16_to_cpu(x) bswap_16(x)
#define __be32_to_cpu(x) bswap_32(x)
#define __be64_to_cpu(x) bswap_64(x)
#define __cpu_to_le32(x) (x)
#else
#define __cpu_to_be16(x) (x)
#define __cpu_to_be32(x) (x)
#define __cpu_to_be64(x) (x)
#define __be16_to_cpu(x) (x)
#define __be32_to_cpu(x) (x)
#define __be64_to_cpu(x) (x)
#define __cpu_to_le32(x) bswap_32(x)
#endif

#define	DEFDMODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#define	DEFFMODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

#define min(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })

#define min_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#define max_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })

extern int chrdev_open(char *modname, char *devpath, uint8_t minor, int *fd);
extern int backed_file_open(char *path, int oflag, uint64_t *size);
extern int set_non_blocking(int fd);

#define zalloc(size)			\
({					\
	void *ptr = malloc(size);	\
	if (ptr)			\
		memset(ptr, 0, size);	\
	else				\
		eprintf("%m\n");	\
	ptr;				\
})

static inline int before(uint32_t seq1, uint32_t seq2)
{
        return (int32_t)(seq1 - seq2) < 0;
}

static inline int after(uint32_t seq1, uint32_t seq2)
{
	return (int32_t)(seq2 - seq1) < 0;
}

/* is s2<=s1<=s3 ? */
static inline int between(uint32_t seq1, uint32_t seq2, uint32_t seq3)
{
	return seq3 - seq2 >= seq1 - seq2;
}

#define shprintf(total, buf, rest, fmt, args...)			\
do {									\
	int len;							\
	len = snprintf(buf, rest, fmt, ##args);				\
	if (len > rest)							\
		goto overflow;						\
	buf += len;							\
	total += len;							\
	rest -= len;							\
} while (0)

extern unsigned long pagesize, pageshift;


/*
 * the latest glibc have a proper sync_file_range definition but
 * most of the distributions aren't shipped with it yet.
*/

#ifndef __NR_sync_file_range
#if defined(__i386)
#define __NR_sync_file_range	314
#elif defined(__x86_64__)
#define __NR_sync_file_range	277
#elif defined(__ia64__)
#define __NR_sync_file_range	1300
#elif defined(__powerpc64__) || defined(__PPC__)
#define __NR_sync_file_range	308
#endif
#endif

#ifndef SYNC_FILE_RANGE_WAIT_BEFORE
#define SYNC_FILE_RANGE_WAIT_BEFORE	1
#define SYNC_FILE_RANGE_WRITE		2
#define SYNC_FILE_RANGE_WAIT_AFTER	4
#endif

extern long int syscall(long int sysno, ...);

static inline int __sync_file_range(int fd, __off64_t offset, __off64_t bytes,
				    unsigned int flags)
{
	int ret;

	ret = syscall(__NR_sync_file_range, fd, offset, bytes, flags);
	if (ret == -EPERM)
		ret = fsync(fd);
	return ret;
}

#endif
