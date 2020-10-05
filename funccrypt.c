#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h> /* PROT_* */
#include <string.h> /* strdup() */

#include "utils.h"

//#define VERBOSE 1

#ifndef __GNUC__
#error "Unknown compiler; Only GCC supports `asm goto'."
#endif

#ifdef __clang__
#error "CLang does not support `asm goto' (yet?)."
#endif

/* Force GCC struct for MingW compilers and pack them,
 * which means the struct is 1-byte aligned.
 */
#define GCC_PACKED __attribute__((packed, gcc_struct))

typedef struct crypt_header {
    uint64_t key;
    uint8_t crypted;
    uint32_t marker;
    uint64_t func_body[0];
} GCC_PACKED crypt_header;

#define CRYPT_FUNC_MAXSIZ 0x100
#define CRYPT_FUNC(fn) \
    crypt_func((void *)fn, 0, NULL, NULL)
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
    CRET_ERROR, CRET_PROLOGUE, CRET_EPILOGUE, CRET_CHECK, CRET_OK,
    CRET_CHECK_ENCRYPTED, CRET_CHECK_PLAIN
} crypt_return;

static const char *crypt_strs[] = {
    "ERROR", "PROLOGUE", "EPILOGUE", "CHECK", "OK", "ENCRYPTED", "PLAIN"
};


static crypt_return crypt_func(void *fn_start, int do_check, struct crypt_header ** const func_crypt_header,
                               size_t * const func_body_size);
static void printHexBuf(uint8_t *buf, size_t siz, size_t chars_per_line);

static int some_fn(int arg0, char *arg1, void *arg2)
{
    CRYPT_PROLOGUE(some_fn);
    int i;
    printf("I'm decrypted .. args: %d, %s, %p\n", arg0, arg1, arg2);
    for (i = 0; i < 32; ++i)
        printf("%d ", i);
    puts("");
    CRYPT_EPILOGUE(some_fn);

    return 0x66;
}

static void another_fn(void)
{
    CRYPT_PROLOGUE(another_fn);
    printf("Another decrypted fn..\n");
    CRYPT_EPILOGUE(another_fn);
}

CRYPT_FNDEF(fndef_fn, void *arg0, int arg1, const char *arg2)
{
    printf("Yet another cryptable fn.. args: %p, %d, %s", arg0, arg1, arg2);
    /* generate some nonsense machine instructions */
    printf("."); printf("."); printf("."); printf(".");
    printf("\n");
}
CRYPT_FNEND(fndef_fn);

static crypt_return crypt_func(void *fn_start, int do_check,
                               struct crypt_header ** const func_crypt_header,
                               size_t * const func_body_size)
{
    size_t i;
    enum crypt_return cret = CRET_ERROR;
    uint8_t *fnbuf = (uint8_t *) fn_start;
    uint8_t *pro, *epi, *mbuf;
    const uint32_t prologue_marker = 0xC0DEC0DE;
    const uint32_t epilogue_marker = 0xCAFECAFE;
    crypt_header *hdr = NULL;
    size_t crypt_size = 0;

    printf("Fn: %p\n", fnbuf);
    for (i = 0; i < CRYPT_FUNC_MAXSIZ; ++i) {
        if (cret == CRET_ERROR &&
            *(uint32_t *) &fnbuf[i] == prologue_marker)
        {
#if VERBOSE
            printf("Prologue Marker: %p\n", &fnbuf[i]);
#endif
            pro = &fnbuf[i];
            cret = CRET_PROLOGUE;
        } else
        if (cret == CRET_PROLOGUE &&
            *(uint32_t *) &fnbuf[i] == epilogue_marker)
        {
#if VERBOSE
            printf("Epilogue Marker: %p\n", &fnbuf[i]);
#endif
            epi = &fnbuf[i];
            cret = CRET_EPILOGUE;
            break;
        }
    }

    if (cret == CRET_EPILOGUE &&
        i >= sizeof *hdr)
    {
        cret = CRET_CHECK;
#if VERBOSE
        printf("Prologue: ");
        printHexBuf(pro - 9, 13, 13);
        printf("Epilogue: ");
        printHexBuf(epi, 4, 4);
#endif
        hdr = (crypt_header *)(pro + sizeof(prologue_marker) - sizeof *hdr);
        crypt_size = epi - (pro + sizeof(prologue_marker));
        if (do_check)
        {
            if (hdr->crypted == 0x00) {
                cret = CRET_CHECK_PLAIN;
            } else {
                cret = CRET_CHECK_ENCRYPTED;
            }
        } else if (i &&
                   (hdr->crypted == 0x00 || hdr->crypted == 0xFF) &&
                   (long int)crypt_size < sysconf(_SC_PAGESIZE))
        {
            mbuf = (uint8_t *)( (long int)hdr & ~(sysconf(_SC_PAGESIZE) - 1) );
            if (!mprotect(mbuf, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE|PROT_EXEC))
            {
                if (hdr->crypted == 0x00) {
                    hdr->crypted = 0xFF;
                    hdr->key = (uint64_t) rand() << 32;
                    hdr->key |= rand();
                } else {
                    hdr->crypted = 0x00;
                }
                for (i = 0; i < crypt_size / 0x8; ++i) {
                    hdr->func_body[i] ^= hdr->key;
                }

                if (!mprotect(mbuf, sysconf(_SC_PAGESIZE), PROT_READ|PROT_EXEC))
                    cret = CRET_OK;
            }
        }
    }

    if (func_crypt_header) {
        *func_crypt_header = hdr;
    }
    if (func_body_size) {
        *func_body_size = crypt_size;
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

static void calcAndPrintEntropy(struct crypt_header * const func_crypt_header,
                                size_t const func_body_size)
{
    printf("Entropy of %p with size %zu: %lf\n", func_crypt_header, func_body_size,
           entropy_from_buffer((uint8_t *)func_crypt_header->func_body, func_body_size));
}

int main(void)
{
    struct crypt_header * hdr = NULL;
    size_t func_body_size = 0;
    crypt_return cret;

    srand(time(NULL));

    cret = crypt_func((void *)some_fn, 1, &hdr, &func_body_size);

    printf("some_fn check return val: %s\n", crypt_strs[cret]);
    calcAndPrintEntropy(hdr, func_body_size);
    printf("some_fn unencrypted:\n");
    printHexBuf((uint8_t *)hdr->func_body, func_body_size, 32);

    if (cret == CRET_CHECK_PLAIN) {
        cret = crypt_func((void *)some_fn, 0, &hdr, &func_body_size);
        assert(cret == CRET_OK);

        printf("some_fn encryption return val: %s\n", crypt_strs[cret]);
        calcAndPrintEntropy(hdr, func_body_size);
        printf("some_fn encrypted:\n");
        printHexBuf((uint8_t *)hdr->func_body, func_body_size, 32);
    }

    puts("\n");

    cret = crypt_func((void *)another_fn, 1, &hdr, &func_body_size);

    printf("another_fn check return val: %s\n", crypt_strs[cret]);
    calcAndPrintEntropy(hdr, func_body_size);
    printf("another_fn unencrypted:\n");
    printHexBuf((uint8_t *)hdr->func_body, func_body_size, 32);

    if (cret == CRET_CHECK_PLAIN) {
        cret = crypt_func((void *)another_fn, 0, &hdr, &func_body_size);
        assert(cret == CRET_OK);

        printf("another_fn return val: %s\n", crypt_strs[cret]);
        calcAndPrintEntropy(hdr, func_body_size);
        printf("another_fn encrypted:\n");
        printHexBuf((uint8_t *)hdr->func_body, func_body_size, 32);
    }

    puts("\n");

    cret = crypt_func((void *)fndef_fn, 1, &hdr, &func_body_size);

    printf("fndef_fn check return val: %s\n", crypt_strs[cret]);
    calcAndPrintEntropy(hdr, func_body_size);
    printf("fndef_fn unencrypted:\n");
    printHexBuf((uint8_t *)hdr->func_body, func_body_size, 32);

    if (cret == CRET_CHECK_PLAIN) {
        cret = crypt_func((void *)fndef_fn, 0, &hdr, &func_body_size);
        assert(cret == CRET_OK);

        printf("fndef_fn return val: %s\n", crypt_strs[cret]);
        calcAndPrintEntropy(hdr, func_body_size);
        printf("fndef_fn encrypted:\n");
        printHexBuf((uint8_t *)hdr->func_body, func_body_size, 32);
    }

    puts("\n");

    printf("\n[output]\n");
    printf("some_fn:\n");
    char * blah = strdup("BLAH");
    printf("return value: 0x%X\n", some_fn(0, blah, NULL));
    free(blah);
    printf("\nanother_fn:\n");
    another_fn();
    printf("\nfndef_fn:\n");
    fndef_fn(NULL, (unsigned int)-1, "TEST");

    return 0;
}
