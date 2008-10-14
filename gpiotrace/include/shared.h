#define EndianSwap32(n) (((((unsigned long) n) << 24) & 0xFF000000) | \
                         ((((unsigned long) n) <<  8) & 0x00FF0000) | \
                         ((((unsigned long) n) >>  8) & 0x0000FF00) | \
                         ((((unsigned long) n) >> 24) & 0x000000FF))

#define EndianSwap16(n) (((((unsigned int) n) << 8) & 0xFF00) | \
                         ((((unsigned int) n) >> 8) & 0x00FF))

/* Endian fixing when reading registers etc. */
#ifdef ARM_MODE
#define EndianFix32(x) x
#define EndianFix16(x) x
#else
#define EndianFix32(x) EndianSwap32(x)
#define EndianFix16(x) EndianSwap16(x)
#endif


