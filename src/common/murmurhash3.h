//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

#include <cstdint>
#include <string>
#include <cstring>

namespace hypocampd {

    extern uint32_t murmurhash (const void * key, int len, uint32_t seed);

    struct MurmurHash {
        uint32_t operator() (const std::string& key, uint32_t seed) const {
            return murmurhash(key.c_str(), key.size(), seed);
        }

        uint32_t operator() (const void* key, int len, uint32_t seed) const {
            return murmurhash(key, len, seed);
        }

        uint32_t operator() (const char* key, uint32_t seed) const {
            return murmurhash(key, strlen(key), seed);
        }
    };

}; // END namespace hypocampd


#endif // _MURMURHASH3_H_

