
#include <cinttypes>

class GcTracker {

};

void gc_fill(int16_t table[0x10000]) {

}


void do_gc() {
    int16_t table[0x10000];
    gc_fill(table);
}