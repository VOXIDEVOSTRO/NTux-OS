#include "partutil.h"

void dump_partitions(DiskOps* ops) {
    uint8_t buf[512];
    if (!ops->read(buf, 0)) return;
    part_entry_t* p = (part_entry_t*)(buf + 446);
    for (int i = 0; i < 4; ++i) {
        kprint("part ");
        kprint_uint(i);
        kprint(": type ");
        kprinthex(p[i].type);
        kprint(" start ");
        kprint_uint(p[i].lba_start);
        kprint(" sectors ");
        kprint_uint(p[i].sectors);
        kprint("\n");
    }
}
