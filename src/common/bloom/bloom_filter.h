#ifndef HYPOTHALD_BLOOM_FILTER_H
#define HYPOTHALD_BLOOM_FILTER_ H

#include <cmath>
#include <limits.h>
#include <vector>
#include <string>
#include <cstring>
#include <common/logger.h>
#include <boost/noncopyable.hpp>
#include "common/murmurhash3.h"

//#include "common/fnvhash.h"

using namespace boost;

namespace hypocampd {

    /*
     * @struct: Policy struct used by BloomFilter class
     * to allocate / deallocate buffer to store bitset
     * via new.
     */
    struct NewCreate {
        static uint8_t* allocate(size_t num_bytes);
        static void deallocate(uint8_t* buffer, size_t size);
    };

    /*
     * @struct: Policy struct used by BloomFilter class
     * to allocate / deallocate buffer to store bitset
     * via mmap operation.
     * Allocation and deallocation via mmap is not
     * thread safe.
     */
    struct MmapCreate {
        static uint8_t* allocate(size_t num_bytes);
        static void deallocate(uint8_t* buffer, size_t size);
    };


    /*
     * @class: BloomFilter<HashT, CreateT>
     *         HashT: Hashing policy, used to create hash values. 
     *                Default is Murmur hash.
     *         CreateT: Creation policy, either using new/mmap/shmget. 
     *                  Default is using 'new' operator.
     *
     * Bloom filter is a statistical probabilistic data structure
     * which is fast and efficient way to determine whether an 
     * entry is present in a particular media, accessing which is many
     * times slower than accessing main memory, for eg: disk access.
     */

    template <
              class HashT = MurmurHash,
              class CreateT = NewCreate
             >
    class BloomFilter : noncopyable {

    using self = BloomFilter<HashT, CreateT>;
    using HashFunc = int (*) (const char*, int);

    public:
        /*
         * @class method: Constructor
         *  estimated_items: Total items estimated to be in the bloom filter
         *  false_prob_rate: Acceptable false positive probability.
         */
        BloomFilter(size_t estimated_items, float false_prob_rate):
                    m_est_items(estimated_items),
                    m_stored_items(0),
                    m_false_prob_rt(false_prob_rate) {

            double num_hashes = -std::log(m_false_prob_rt) / std::log(2);
            m_num_hash_funcs = static_cast<size_t>(num_hashes);

            m_num_bits = static_cast<size_t>(m_est_items * m_num_hash_funcs / std::log(2)); 
    
            m_num_bytes = (m_num_bits/CHAR_BIT) + (m_num_bits%CHAR_BIT ? 1 : 0);

            // Allocate memory for holding bitset
            // TODO Make allocation exception safe
            FINFO("%s: %d", "Bytes needed = ", m_num_bytes);
            FINFO("%s: %d", "Hash functions = ", m_num_hash_funcs);
            mp_bloom_buf = CreateT::allocate(m_num_bytes);

            if (mp_bloom_buf == nullptr) throw;

            memset(mp_bloom_buf, 0, m_num_bytes);
        }

        /*
         * @class method: Destructor
         */
        ~BloomFilter() {
            CreateT::deallocate(mp_bloom_buf, get_size_in_bytes());
        }

        /*
         * @class method: Inserts data into bloom filter
         *                Sets the bit returned by the hash function.
         * const void* key: Pointer to the key/buffer
         * size_t key_len : Size of the key in bytes
         */

        void insert(const void* key, size_t key_len) noexcept {
            uint32_t seed = key_len;

            for (int i = 0; i < m_num_hash_funcs; i++) {
                seed = m_hash(key, key_len, seed) % m_num_bits;
                mp_bloom_buf[seed / CHAR_BIT] = (1 << (seed % CHAR_BIT));
            }
            m_stored_items++;
        }

        /* 
         * @class method: Overloaded insert method for strings
         */
        void insert(const std::string& key) noexcept {
            insert(key.c_str(), key.size());
        }

        /*
         * @class method: Checks whether the supplied key
         *                is present or seen by the bloom filter.
         * const void* key: The key to be checked
         * size_t len: Length of the key
         *
         * Method returns true if already seen or false if not seen.
         */

        bool can_have(const void* key, const size_t key_len) const noexcept {
            uint32_t seed = key_len;

            for (int i = 0; i < m_num_hash_funcs; i++ ) {
                seed = m_hash(key, key_len, seed) % m_num_bits;
                uint8_t byte_pos = mp_bloom_buf[seed / CHAR_BIT]; 
                uint8_t mask = (1 << (seed % CHAR_BIT));

                if ((byte_pos & mask) == 0) return false;
            }
            return true;
        }

        /*
         * @class method: Overload for std::string
         */

        bool can_have(const std::string& key) const noexcept {
            can_have(key.c_str(), key.size());
        }

        size_t get_estimated_items() const noexcept {
            return m_est_items;
        }

        size_t get_stored_items() const noexcept {
            return m_stored_items;
        }

        size_t get_size_in_bits() const noexcept {
            return m_num_bits;
        }

        size_t get_size_in_bytes() const noexcept {
            return m_num_bytes;
        }

    private:
        /* Private data members */
        // Estimated total number of items
        size_t m_est_items      = 0;
        // Actual number of items in filter
        size_t m_stored_items   = 0;
        // False positive rate
        float m_false_prob_rt   = 0.0;
        // No. of hash functions used
        size_t m_num_hash_funcs = 0;
        // Size of filter in no. of bits
        size_t m_num_bits       = 0;
        // Size of filter in no. of bytes
        size_t m_num_bytes      = 0;
        // Bloom filter buffer pointer
        uint8_t* mp_bloom_buf   = nullptr;

        // Hash caluculater
        HashT m_hash;
    };


}; // END Namespace hypocampd

#endif // HYPOTHALD_BLOOM_FILTER_
