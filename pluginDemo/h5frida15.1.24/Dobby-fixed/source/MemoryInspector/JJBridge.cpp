#include "MemScan.h"
#include <stdint.h>
#include <stddef.h>

extern "C" {
    bool JJReadBytes(void* enginePtr, void* buf, uint64_t addr, size_t len) {
        if (!enginePtr || !buf || len == 0) return false;
        JJMemoryEngine* e = reinterpret_cast<JJMemoryEngine*>(enginePtr);
        uint8_t* out = reinterpret_cast<uint8_t*>(buf);
        for (size_t i = 0; i < len; i++) {
            // Read single byte using existing public JJReadMemory with UByte type
            if (!e->JJReadMemory((void*)(out + i), addr + i, JJ_Search_Type_UByte))
                return false;
        }
        return true;
    }
}