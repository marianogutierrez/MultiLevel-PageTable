#include <stdint.h>
#include <stdio.h>

typedef struct BYUADDRESSTRACE
{
  uint32_t addr;
  unsigned char reqtype;
  unsigned char size;
  unsigned char attr;
  unsigned char proc;
  uint32_t time;
} p2AddrTr;

typedef enum {
  UNKNOWN,
  LITTLE,       /* native format of trace file */
  BIG           /* foreign format, will need to translate */
} ENDIAN;


/* NextAddress - Fetch the next address from the trace.
 * See byu_tracereader.c for details.
 */
int NextAddress(FILE *trace_file, p2AddrTr *addr_ptr);

/* reqtype values */
#define FETCH                   0x00    // instruction fetch
#define MEMREAD                 0x01    // memory read
#define MEMREADINV              0x02    // memory read and invalidate
#define MEMWRITE                0x03    // memory write

#define IOREAD                  0x10    // IO read
#define IOWRITE                 0x11    // IO Write

#define DEFERREPLY              0x20    // deferred reply
#define INTA                    0x21    // interrupt acknowledge
#define CNTRLAGNTRES            0x22    // central agent response
#define BRTRACEREC              0x23    // branch trace record

#define SHUTDOWN                0x31    // shutdown
#define FLUSH                   0x32    // flush
#define HALT                    0x33    // halt
#define SYNC                    0x34    // sync
#define FLUSHACK                0x35    // acknowledge flush
#define STOPCLKACK              0x36    // acknowledge stop clock
#define SMIACK                  0x37    // acknowledge SMI mode