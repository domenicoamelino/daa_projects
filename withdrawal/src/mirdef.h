#define __int64 long long
#define MIRACL 32
#define MR_LITTLE_ENDIAN    /* This may need to be changed        */
#define mr_utype int
                            /* the underlying type is usually int *
                             * but see mrmuldv.any                */
#define mr_unsign32 unsigned int
                            /* 32 bit unsigned type               */
#define MR_IBITS      32    /* bits in int  */
#define MR_LBITS      32    /* bits in long */
//#define MR_FLASH      52
                            /* delete this definition if integer  *
                             * only version of MIRACL required    */
                            /* Number of bits per double mantissa */
//#define DAMEL_RAND_NULL
#define MR_NORAND
#define mr_dltype __int64   /* ... or long long for Unix/Linux */
#define mr_unsign64 unsigned __int64
#define MR_NOASM

#define MAXBASE ((mr_small)1<<(MIRACL-1))
