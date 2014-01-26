//
// http://www.splicd.com/AAEJedBQk7s/19/43
//
//#define GEN_FILTER 1

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

/* config options */
/* 2^FILTER_SIZE is the size of the filter in bits, i.e.,
 * size 20 = 2^20 bits = 1 048 576 bits = 131 072 bytes = 128 KB */
#define FILTER_SIZE 23
#define NUM_HASHES 7
#define WORD_BUF_SIZE 32

#define FILTER_SIZE_BYTES (1 << (FILTER_SIZE - 3))
#define FILTER_BITMASK ((1 << FILTER_SIZE) - 1)

/* hash functions */
unsigned int RSHash  (unsigned char *, unsigned int);
unsigned int DJBHash (unsigned char *, unsigned int);
unsigned int FNVHash (unsigned char *, unsigned int);
unsigned int JSHash  (unsigned char *, unsigned int);
unsigned int PJWHash (unsigned char *, unsigned int);
uint32_t SFHash      (unsigned char *, unsigned int);
unsigned int SDBMHash(unsigned char *, unsigned int);
unsigned int DEKHash (unsigned char *, unsigned int);

/* helper functions */
void err(char *msg, ...);
void load_words(unsigned char[], char *);
void insert_word(unsigned char[], char *);
unsigned int in_dict(unsigned char[], char *);
void get_hashes(unsigned int[], char *);


int main(void)
{

#ifdef GEN_FILTER
    printf("/* Bloom filter generated with 2^%d filter size.\n", FILTER_SIZE);
    printf("   And %d hash functions. */\n", NUM_HASHES);

#include "dict.c"

    char *p;
    unsigned char filter[FILTER_SIZE_BYTES];

    int n;
    n = 0;
    while (n<234936) {
        if ((p=strchr(dict[n], '\r'))) *p='\0';
        if ((p=strchr(dict[n], '\n'))) *p='\0';
        insert_word(filter, dict[n]);
        n++;
    }

    n = 0;
    printf("unsigned char filter[FILTER_SIZE_BYTES] = {");
    for (n = 0; n < sizeof(filter); n++)
        printf("0x%x, ", filter[n]);
    printf("};\n");
#else

#include "filter.c"

    register unsigned int n;

#endif

#define I_BUFSIZE (int)(4096 * 4)
#define O_BUFSIZE (int)(4096 * 2)

    char output[O_BUFSIZE];
    setvbuf(stdout, output, _IOFBF, sizeof(output));

    char wordbuf[40];
    register unsigned int wordbuf_i = 0;

    char checkbuf[40];
    register unsigned int checkbuf_i = 0;

    char * buf = malloc(I_BUFSIZE);

    char c;

    n = fread(buf, 1, I_BUFSIZE, stdin);
    register unsigned int i;
    for (i=0; i<n; i++) {
        c = buf[i];

        if (c == '\0') {
            break;
        }

        wordbuf[wordbuf_i] = c;

        checkbuf[checkbuf_i] = c | (char)0x20;

        if (c == ' ' || c == '\n') {
            if (wordbuf_i != 0) {
                wordbuf[wordbuf_i] = '\0';
                checkbuf[checkbuf_i] = '\0';

                // If comparing uint32_t.
                checkbuf[checkbuf_i + 1] = '\0';
                checkbuf[checkbuf_i + 2] = '\0';
                checkbuf[checkbuf_i + 3] = '\0';

                if (!in_dict(filter, checkbuf)) {
                //if (1) {
                    putc('<', stdout);
                    fwrite(wordbuf, wordbuf_i, 1, stdout);
                    putc('>', stdout);
                } else {
                    // Print the word.
                    fwrite(wordbuf, wordbuf_i, 1, stdout);
                }

                // Compiler probably already does this
                wordbuf_i ^= wordbuf_i;
                checkbuf_i ^= checkbuf_i;
            }

            putc(c, stdout);
        } else {
            wordbuf_i++;
            checkbuf_i++;
        }
    }

    fflush(stdout);
    return 0;
}


void err(char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(-1);
}


void insert_word(unsigned char filter[], char *str)
{
    unsigned int hash[NUM_HASHES];
    int i;

    get_hashes(hash, str);

    for (i = 0; i < NUM_HASHES; i++) {
        /* xor-fold the hash into FILTER_SIZE bits */
        hash[i] = (hash[i] >> FILTER_SIZE) ^
                  (hash[i] & FILTER_BITMASK);
        /* set the bit in the filter */
        filter[hash[i] >> 3] |= 1 << (hash[i] & 7);
    }
}


inline unsigned int in_dict(unsigned char filter[], char *str)
{
    unsigned int hash[NUM_HASHES];
    register unsigned int i;

    get_hashes(hash, str);

    for (i = 0; i < NUM_HASHES; i++) {
        hash[i] = (hash[i] >> FILTER_SIZE) ^
                  (hash[i] & FILTER_BITMASK);
        if (!(filter[hash[i] >> 3] & (1 << (hash[i] & 7))))
            return 0;
    }

    return 1;
}

void get_hashes(unsigned int hash[], char *in)
{
    unsigned char *str = (unsigned char *)in;
    register unsigned int pos = strlen(in);
    hash[0] = RSHash  (str, pos);
    hash[1] = DJBHash (str, pos);
    hash[2] = FNVHash (str, pos);
    hash[3] = JSHash  (str, pos);
    /* hash[4] = PJWHash (str, pos); */
    hash[4] = SFHash (str, pos);
    hash[5] = SDBMHash(str, pos);
    hash[6] = DEKHash (str, pos);
}

inline unsigned int RSHash(unsigned char *str, unsigned int len)
{
    unsigned int b    = 378551;
    unsigned int a    = 63689;
    unsigned int hash = 0;
    unsigned int i    = 0;

    for(i = 0; i < len; str++, i++)
    {
        hash = hash * a + (*str);
        a    = a * b;
    }

    return hash;
}

inline unsigned int JSHash(unsigned char *str, unsigned int len)
{
    unsigned int hash = 1315423911;
    unsigned int i    = 0;

    for(i = 0; i < len; str++, i++)
    {
        hash ^= ((hash << 5) + (*str) + (hash >> 2));
    }

    return hash;
}

inline unsigned int PJWHash(unsigned char *str, unsigned int len)
{
    const unsigned int BitsInUnsignedInt = (unsigned int)(sizeof(unsigned int) * 8);
    const unsigned int ThreeQuarters     = (unsigned int)((BitsInUnsignedInt  * 3) / 4);
    const unsigned int OneEighth         = (unsigned int)(BitsInUnsignedInt / 8);
    const unsigned int HighBits          = (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
    unsigned int hash              = 0;
    unsigned int test              = 0;
    unsigned int i                 = 0;

    for(i = 0; i < len; str++, i++)
    {
        hash = (hash << OneEighth) + (*str);

        if((test = hash & HighBits)  != 0)
        {
            hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
        }
    }

    return hash;
}

inline unsigned int SDBMHash(unsigned char *str, unsigned int len)
{
    unsigned int hash = 0;
    unsigned int i    = 0;

    for(i = 0; i < len; str++, i++)
    {
        hash = (*str) + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

inline unsigned int DJBHash(unsigned char *str, unsigned int len)
{
    unsigned int hash = 5381;
    unsigned int i    = 0;

    for(i = 0; i < len; str++, i++)
    {
        hash = ((hash << 5) + hash) + (*str);
    }

    return hash;
}

inline unsigned int DEKHash(unsigned char *str, unsigned int len)
{
    unsigned int hash = len;
    unsigned int i    = 0;

    for(i = 0; i < len; str++, i++)
    {
        hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
    }
    return hash;
}

inline unsigned int FNVHash(unsigned char *str, unsigned int len)
{
    const unsigned int fnv_prime = 0x811C9DC5;
    unsigned int hash      = 0;
    unsigned int i         = 0;

    for(i = 0; i < len; str++, i++)
    {
        hash *= fnv_prime;
        hash ^= (*str);
    }

    return hash;
}

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

// SuperFastHash (sounds legit)
// http://www.azillionmonkeys.com/qed/hash.html
uint32_t SFHash (unsigned char * data, unsigned int len) {
    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (; len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
    case 3:
        hash += get16bits (data);
        hash ^= hash << 16;
        hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
        hash += hash >> 11;
        break;
    case 2:
        hash += get16bits (data);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1:
        hash += (signed char)*data;
        hash ^= hash << 10;
        hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

uint32_t toupper2(uint32_t eax)
{
    uint32_t ebx = (0x7f7f7f7ful & eax) + 0x05050505ul;
    ebx = (0x7f7f7f7ful & ebx) + 0x1a1a1a1aul;
    ebx = ((ebx & ~eax) >> 2 ) & 0x20202020ul;
    return eax - ebx;
}
//
// http://www.splicd.com/gwx87LLZib8/20/30
//
