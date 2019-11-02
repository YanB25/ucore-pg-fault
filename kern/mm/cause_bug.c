#include <defs.h>

// #define NO_BUG 135084
#define HAS_BUG 135085

/* alloc a LONG array */
/* END could not be larger than 0xc01b5000 */
uint32_t array[HAS_BUG];