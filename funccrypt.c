#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h> /* PROT_* */


/* Force GCC struct for MingW compilers and pack them,
 * which means the struct is 1-byte aligned.
 */
#define GCC_PACKED __attribute__((packed, gcc_struct))

typedef struct crypt_header {
    uint64_t key;
    uint8_t crpyted;
    uint32_t marker;
    uint64_t func_body[0];
} GCC_PACKED crypt_header;

#define CRYPT_FUNC_MAXSIZ 0x100
#define CRYPT_FUNC(fn) \
    crypt_func((void *)fn)
#define CRYPT_PROLOGUE(fn) \
    crypt_return __cr; \
    { \
        __cr = CRYPT_FUNC(fn); \
        if (__cr != CRET_OK) \
            asm volatile goto("jmp %l0          \n" \
                : : : : cr_epilogue); \
        asm volatile goto("jmp %l0          \n" \
            : : : : cr_prologue); \
        asm volatile( \
            ".byte " \
            /* key: */     "0x00, 0x00, 0x00, 0x00," \
                           "0x00, 0x00, 0x00, 0x00," \
            /* crypted: */ "0x00," \
            /* marker: */  "0xDE, 0xC0, 0xDE, 0xC0;  \n" \
        ); \
    } \
    cr_prologue: {
#define CRYPT_EPILOGUE(fn) \
    } { \
        asm volatile goto("jmp %l0          \n" \
            : : : : cr_epilogue); \
        asm volatile( \
            ".byte " \
            /* Insert encryption gap, so we can find the end marker,
             * while the function body is encrypted.
             */ \
            "0x00, 0x00, 0x00, 0x00," \
            "0x00, 0x00, 0x00, 0x00," \
            /* marker: */   "0xFE, 0xCA, 0xFE, 0xCA;  \n" \
        ); \
    } \
    cr_epilogue: \
    CRYPT_FUNC(fn);

#define CRYPT_FNDEF(name, ...) \
    void name( __VA_ARGS__ ) { \
        CRYPT_PROLOGUE( name );
#define CRYPT_FNEND(name) \
        CRYPT_EPILOGUE(name); \
    }


typedef enum crypt_return {
    CRET_ERROR, CRET_PROLOGUE, CRET_EPILOGUE, CRET_CHECK, CRET_OK
} crypt_return;

static const char *crypt_strs[] = {
    "ERROR", "PROLOGUE", "EPILOGUE", "CHECK", "OK"
};


static crypt_return crypt_func(void *fn_start);
static void printHexBuf(uint8_t *buf, size_t siz, size_t chars_per_line);

static int crypted_fn(int arg0, char *arg1, void *arg2)
{
    CRYPT_PROLOGUE(crypted_fn);
    int i;
    printf("I'm decrypted ..\n");
    for (i = 0; i < 32; ++i)
        printf("%d ", i);
    puts("");
    CRYPT_EPILOGUE(crypted_fn);

    return 0x66;
}

static void crypted_fn2(void)
{
    CRYPT_PROLOGUE(crypted_fn2);
    printf("Another decrypted fn..\n");
    CRYPT_EPILOGUE(crypted_fn2);
}

CRYPT_FNDEF(crypted_fn3, void *arg0, int arg1, const char *arg2)
{
    printf("Yet another cryptable fn.. ");
    /* generate some nonsense machine instructions */
    printf("."); printf("."); printf("."); printf(".");
    printf("\n");
}
CRYPT_FNEND(crypted_fn3);

static crypt_return crypt_func(void *fn_start)
{
    size_t i;
    enum crypt_return cret = CRET_ERROR;
    uint8_t *fnbuf = (uint8_t *) fn_start;
    uint8_t *pro, *epi, *mbuf;
    const uint32_t prologue_marker = 0xC0DEC0DE;
    const uint32_t epilogue_marker = 0xCAFECAFE;
    crypt_header *hdr;
    size_t crypt_size;

    printf("Fn: %p\n", fnbuf);
    for (i = 0; i < CRYPT_FUNC_MAXSIZ; ++i) {
        if (cret == CRET_ERROR &&
            *(uint32_t *) &fnbuf[i] == prologue_marker)
        {
            printf("Prologue Marker: %p\n", &fnbuf[i]);
            pro = &fnbuf[i];
            cret = CRET_PROLOGUE;
        } else
        if (cret == CRET_PROLOGUE &&
            *(uint32_t *) &fnbuf[i] == epilogue_marker)
        {
            printf("Epilogue Marker: %p\n", &fnbuf[i]);
            epi = &fnbuf[i];
            cret = CRET_EPILOGUE;
            break;
        }
    }

    if (cret == CRET_EPILOGUE &&
        i >= sizeof *hdr)
    {
        cret = CRET_CHECK;
#if 0
        printf("Prologue: ");
        printHexBuf(pro - 9, 13, 13);
        printf("Epilogue: ");
        printHexBuf(epi, 4, 4);
#endif
        hdr = (crypt_header *)(pro + sizeof(prologue_marker) - sizeof *hdr);
        crypt_size = epi - (pro + sizeof(prologue_marker)) - 1;

        if (i &&
            (hdr->crpyted == 0x00 || hdr->crpyted == 0xFF) &&
            (long int)crypt_size < sysconf(_SC_PAGESIZE))
        {
            mbuf = (uint8_t *)( (long int)hdr & ~(sysconf(_SC_PAGESIZE) - 1) );
            if (!mprotect(mbuf, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE|PROT_EXEC))
            {
                if (hdr->crpyted == 0x00) {
                    hdr->crpyted = 0xFF;
                    hdr->key = (uint64_t) rand() << 32;
                    hdr->key |= rand();
                }
                for (i = 0; i < crypt_size / 0x8; ++i) {
                    hdr->func_body[i] ^= hdr->key;
                }

                if (!mprotect(mbuf, sysconf(_SC_PAGESIZE), PROT_READ|PROT_EXEC))
                    cret = CRET_OK;
            }
        }
    }

    return cret;
}

static void printHexBuf(uint8_t *buf, size_t siz, size_t chars_per_line)
{
    size_t i;

    for (i = 0; i < siz; ++i) {
        printf("%02X ", buf[i]);
        if ((i+1) % chars_per_line == 0)
            printf("\n");
    }
    if ((i) % chars_per_line != 0)
        printf("\n");
}

int main(void)
{
    srand(time(NULL));

    printf("Before Encryption:\n");
    printf("crypted_fn:\n");
    printHexBuf((uint8_t *)crypted_fn, 160, 32);
    printf("crypted_fn2:\n");
    printHexBuf((uint8_t *)crypted_fn2, 160, 32);
    printf("crypted_fn3:\n");
    printHexBuf((uint8_t *)crypted_fn3, 160, 32);

    printf("\nAfter Encryption:\n");
    printf("crypted_fn return val: %s\n",
           crypt_strs[ crypt_func((void *)crypted_fn) ]);
    printf("crypted_fn2 return val: %s\n",
           crypt_strs[ crypt_func((void *)crypted_fn2) ]);
    printf("crypted_fn3 return val: %s\n",
           crypt_strs[ crypt_func((void *)crypted_fn3) ]);

    printf("crypted_fn:\n");
    printHexBuf((uint8_t *)crypted_fn, 160, 32);
    printf("crypted_fn2:\n");
    printHexBuf((uint8_t *)crypted_fn2, 160, 32);
    printf("crypted_fn3:\n");
    printHexBuf((uint8_t *)crypted_fn3, 160, 32);

    printf("\noutput:\n");
    printf("crypted_fn: 0x%X\n", crypted_fn(0, NULL, NULL));
    crypted_fn2();
    crypted_fn3(NULL, (unsigned int)-1, "TEST");

    return 0;
}
