#ifndef HYPOCAMPD_HASH_MAP_H
#define HYPOCAMPD_HASH_MAP

// This hash map is kind of similar to Redis dict

#include "common/murmurhash3.h"

#define HASH_MAP_MIN_ENTRIES 512 // Should be always power of two

namespace hypocampd {

enum result_t {
    HASH_OK = 0,
    HASH_ERR,
};

struct Entry {
    AObject key; 
    union {
        AObject value;
        uint64_t u64;
        int64_t s64;
        double d;
    } val;

    Entry* next = nullptr;
};

class HashTable {
public:
    HashTable(unsigned long size = HASH_MAP_MIN_ENTRIES):
                                m_size(size),
                                m_sizemask(m_size - 1),
                                mp_table(new Entry*[size])
    {
    }

    // Add an element to the hash table
    result_t add(const AObject&& key, const AObject&& val);
  
    // Low level add. This function adds the entry but 
    // instead of setting a value to the entry, it returs the
    // entry itself. the user is resposible to set the entry in 
    // the returned entry. 
    // This is basically used to take advantage of the union field
    // inside Entry.
    // The user in this case can directly store unsigned/signed/double
    // numeric value instead of storing it as a pointer.
    // TODO: Check if this functionality can be provided by the class itself.
    Entry* add_raw(const AObject&& key); 

    Entry* find(const AObject&& key);

    AObject fetch_value(); 
private:
    unsigned long m_size     = 0;
    unsigned long m_sizemask = 0;
    unsigned long m_used     = 0;
    std::unique_ptr<Entry*[]> mp_table;
};

}; // End namespace hypocampd

#endif
