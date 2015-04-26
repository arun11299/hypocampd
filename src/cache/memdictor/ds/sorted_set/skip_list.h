#ifndef SKIP_LIST_INCLUDED
#define SKIP_LIST_INCLUDED

#include <memory>
#include <type_traits>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>

namespace hypocampd {

// Maximum possible height for the skip list
// To change it, change the below static value
static const uint8_t MAX_HEIGHT = 16;

/* HeightHelper: A helper struct for calculating
 * at what level or height the element needs to be placed
 * in the skip list
 * Courtesy: http://www.drdobbs.com/cpp/skip-lists-in-c/184403579?pgno=1
 */
struct HeightHelper {
public:
    /*
     * Constructor: Takes max height and probability
     * as the input parameters.
     */
    HeightHelper(uint8_t height, float prob):
            m_height(height), 
            m_probability(prob) {
        srand (static_cast <unsigned> (time(0)));
    }

    float random(float range) const noexcept {
        return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/range));
    }

    // Ouputs the height or level at which the element
    // needs to be put.
    // If probability is configured as 0.5, then
    // 50 % of the times or the entries will be at level 1
    // 25% on level 2, 12.5% on level 3, like that...
    uint8_t new_height() const noexcept {
        uint8_t tmp_ht = 0;
        while ( (random(1.0) < m_probability) &&
                tmp_ht < m_height - 1) {
            tmp_ht++;
        }
        return tmp_ht;
    }

private:
    uint8_t m_height;
    float   m_probability;
};


// Forward decl
template <typename Key_T, typename Value_T>
class SkipListImpl;

/*
 * SkipListNode: Class representing a single element in 
 *               a skip list.
 * Key_T   :- Actual data type of Key. Should not be a pointer.
 * Value_T :- Actual data type of value. Should not be pointer.
 *
 * SkipListNode owns the resources it holds.
 */

template <typename Key_T, typename Value_T>
class SkipListNode {
public:
    using self_type = SkipListNode<Key_T, Value_T>;
    // TODO: Remove this friendness
    //friend class SkipList<Key_T, Value_T>;

    // This is a sink constructor.
    // Keys and values allocated outside this class
    // will be owned by this class i.e clients need
    // not or should not deallocate the memory aloocated
    // by them for the Key and value
    SkipListNode(const Key_T* key, const Value_T* value, 
                 uint8_t height): mp_key(std::unique_ptr<const Key_T>{key})
                                , mp_value(std::unique_ptr<const Value_T>{value})
                                , m_height(height)
                                , mp_fwd_nodes(new self_type* [m_height]) 
    {
        // Key_T and Value_T must be basic types
        // w/o * and &.
        assert(!std::is_pointer<Key_T>::value &&
               !std::is_pointer<Value_T>::value);

        for (int i = 0; i < m_height; ++i) {
            mp_fwd_nodes[i] = nullptr;        
        }
    }

    
    // Contructor number 2.
    // Takes in just the height as parameter
    SkipListNode(uint8_t height): m_height(height)
                                , mp_fwd_nodes(new self_type* [m_height])
    {
        // Key_T and Value_T must be basic types
        // w/o * and &
        assert(!std::is_pointer<Key_T>::value &&
               !std::is_pointer<Value_T>::value);

        for (int i = 0; i < m_height; ++i) {
            mp_fwd_nodes[i]  = nullptr;
        }
    }

    // Returns the Key value
    const Key_T* get_key() const noexcept {
        return mp_key.get();
    }

    // Returns the value
    const Value_T* get_value() const noexcept {
        return mp_value.get();
    }

    // Returns both key and value in std::pair
    std::pair<const Key_T*, const Value_T*>
    get_key_value() const noexcept {
        return make_pair(get_key(), get_value());
    }

    // Returns the height at which this node is present
    uint8_t get_height() const noexcept {
        return m_height;
    }

private:
    std::unique_ptr<const Key_T> mp_key     = nullptr;
    std::unique_ptr<const Value_T> mp_value = nullptr;
    uint8_t m_height;

public: // I hate doing this !! This should be private !!
    // Pointers/link to other nodes at different levels
    std::unique_ptr<self_type*[]> mp_fwd_nodes;
};




/* SkipListImpl: This is the main class that exposes
 *           the API's for interacting with skip list.
 */

template <typename Key_T, typename Value_T>
class SkipListImpl {
public:
    using self_type = SkipListImpl<Key_T, Value_T>;
    using node_type = SkipListNode<Key_T, Value_T>;

public:
    SkipListImpl(uint8_t height, float prob, const Key_T* max_key):
                m_max_height(height),
                mp_head(new node_type(m_max_height)),
                mp_tail(new node_type(max_key, nullptr, m_max_height)),
                mp_hthelper(new HeightHelper(height, prob))
    {
        assert(m_max_height <= MAX_HEIGHT);
        // Point head to tail at all levels 
        for (int i = 0; i < m_max_height; ++i) {
            mp_head->mp_fwd_nodes[i] = mp_tail.get();
        }
    }

    // Get the head of the list
    const node_type* get_head() const noexcept {
        return mp_head.get();
    } 

    // Get the tail of the list
    const node_type* get_tail() const noexcept {
        return mp_tail.get();
    }

    // Inserts the key with value into the skip list.
    // Return type: bool
    // true : if insertion was successfull
    // false: If duplicate key was found
    bool insert(const Key_T* key, const Value_T* val);
   
    // Removes the node with Key 'key'     
    // Return type: bool
    // true: If element was successfully removed
    // false: If element was not present
    bool remove(const Key_T* key);

    // Finds/searches/retrieves the value keyed by
    // 'key'
    // Return type: Value object
    // Returns the actual value if found OR
    // returns the default constructed value object.
    Value_T find(const Key_T* key);


    // Find the range of entries lying between
    // key_1 and key_2
    std::pair<const node_type*, const node_type*>
    find_range(const Key_T* key_1, const Key_T* key_2);


    // Dump the skip list in layered format
    // Only for debugging purpose.
    void __dump__();

private:
    // Helper function to find the nearest node (usually the one before)
    // to the node having the key.
    void find_nearest(const Key_T* key, const Key_T*& cmp_key, node_type*& tmp, 
                      std::array<node_type*, MAX_HEIGHT>& tracker);

private:
    uint8_t m_max_height;
    std::unique_ptr<node_type> mp_head;
    std::unique_ptr<node_type> mp_tail;
    uint8_t m_curr_height = 0;
    std::unique_ptr<HeightHelper> mp_hthelper = nullptr;
};


template <typename Key_T, typename Value_T>
void SkipListImpl<Key_T, Value_T>::find_nearest(const Key_T* key, const Key_T*& cmp_key,
                        node_type*& tmp,
                        std::array<node_type*, MAX_HEIGHT>& tracker)
{
    for (int i = m_curr_height; i >= 0; i--) {
        // Get the key from the next node at level 'i'
        cmp_key = tmp->mp_fwd_nodes[i]->get_key();

        while (*key < *cmp_key && 
                tmp->mp_fwd_nodes[i] != mp_tail.get())
        {
            tmp = tmp->mp_fwd_nodes[i];
            cmp_key =  tmp->mp_fwd_nodes[i]->get_key();
        }
        // Track the node location at level 'i'
        tracker[i] = tmp;
    } 

    // Go to the bottom level
    tmp = tmp->mp_fwd_nodes[0];
    cmp_key = tmp->get_key();

    return;
}


template <typename Key_T, typename Value_T>
bool SkipListImpl<Key_T, Value_T>::insert(const Key_T* key, const Value_T* val)
{
    node_type* tmp = mp_head.get(); 
    const Key_T* cmp_key = nullptr;
    std::array<node_type*, MAX_HEIGHT> tracker{};

    find_nearest(key, cmp_key, tmp, tracker);

    if (*key == *cmp_key) {
        // Duplicate Key
        return false;
    } else {
        // Get the level at which insert needs to be done
        uint8_t lvl = mp_hthelper->new_height();

        // Create new levels above the current level
        if (lvl > m_curr_height) {
            for (int i = m_curr_height + 1; i <= lvl; i++) {
                tracker[i] = mp_head.get();
            }
            // Update the current level
            m_curr_height = lvl;
        }

        // Insert the node
        tmp = new node_type(key, val, lvl + 1);

        for (int j = 0; j <= lvl; j++) {
            tmp->mp_fwd_nodes[j] = tracker[j]->mp_fwd_nodes[j];
            tracker[j]->mp_fwd_nodes[j] = tmp;
        }
    }

    return true;
}


template <typename Key_T, typename Value_T>
bool SkipListImpl<Key_T, Value_T>::remove(const Key_T* key) 
{
    node_type* tmp = mp_head.get();
    const Key_T* cmp_key = nullptr;
    std::array<node_type*, MAX_HEIGHT> tracker{};

    find_nearest(key, cmp_key, tmp, tracker);

    if (*key == *cmp_key) {

        for (int i = 0; i <= m_curr_height; i++) {
            if (tracker[i]->mp_fwd_nodes[i] != tmp) break;
            tracker[i]->mp_fwd_nodes[i] = tmp->mp_fwd_nodes[i];
        }
        delete tmp;

        while ( (m_curr_height > 0) &&
                (mp_head->mp_fwd_nodes[m_curr_height] == mp_tail.get()) ) {
            m_curr_height--;
        }

        return true;

    } else {

        return false;
    }
}


template <typename Key_T, typename Value_T>
Value_T SkipListImpl<Key_T, Value_T>::find(const Key_T* key)
{
    node_type* tmp = mp_head.get();
    const Key_T* cmp_key = nullptr;
    std::array<node_type*, MAX_HEIGHT> tracker{};

    find_nearest(key, cmp_key, tmp, tracker);

    if (*key == *cmp_key) {
        return *(tmp->get_value());
    } else {
        return Value_T();
    }
}



template <typename Key_T, typename Value_T>
std::pair<const SkipListNode<Key_T, Value_T>*, const SkipListNode<Key_T, Value_T>*>
SkipListImpl<Key_T, Value_T>::find_range(const Key_T* key_1, const Key_T* key_2)
{
    node_type* tmp = mp_head.get();
    const Key_T* cmp_key = nullptr;
    std::array<node_type*, MAX_HEIGHT> tracker1{};

    // Doing 2 find_nearest should be better in
    // average case

    find_nearest(key_1, cmp_key, tmp, tracker1);

    node_type* tmp2 = mp_head.get();
    cmp_key = nullptr;
    std::array<node_type*, MAX_HEIGHT> tracker2{}; 
    
    find_nearest(key_2, cmp_key, tmp2, tracker2);

    return make_pair(const_cast<const node_type*>(tmp),
                     const_cast<const node_type*>(tmp2));
}



template <typename Key_T, typename Value_T>
void SkipListImpl<Key_T, Value_T>::__dump__() 
{
    int tmp_ht = m_curr_height;

    while (tmp_ht >= 0) {
        node_type* tmp = mp_head->mp_fwd_nodes[tmp_ht];

        while ( tmp != mp_tail.get() ) {
            std::cout << *tmp->get_key() << " ";
            tmp = tmp->mp_fwd_nodes[tmp_ht];
        }
        std::cout << std::endl;

        tmp_ht--;
    }

    return;
}



template <typename Key_T, typename Value_T>
class sl_iterator {
public:
    using node_type = SkipListNode<Key_T, Value_T>;
    using self_type = sl_iterator<Key_T, Value_T>;

    sl_iterator(const node_type* node): mp_node(node) {}

    bool operator== (const self_type& other) {
        return mp_node == other.mp_node;
    }

    bool operator!= (const self_type& other) {
        return mp_node != other.mp_node;
    }

    const node_type& operator*() const noexcept {
        return *(this->operator->());
    }

    const node_type* operator->() const noexcept {
        return mp_node;
    }

    // Pre-increment operator
    self_type& operator++() noexcept {
        mp_node = mp_node->mp_fwd_nodes[0];
        return *this;
    } 

    // Post-increment operator
    self_type operator++(int) {
        self_type tmp(*this);
        this->mp_node = this->mp_node->mp_fwd_nodes[0];
        return tmp;
    }

private:
    const node_type* mp_node;
};


template <typename Key_T, typename Value_T>
class SkipList : public SkipListImpl<Key_T, Value_T> {
public:
    using iterator = sl_iterator<Key_T, Value_T>;
    using impl_type = SkipListImpl<Key_T, Value_T>;

    SkipList(uint8_t height, float prob, const Key_T* max_key):
                impl_type(height, prob, max_key) {}

    iterator begin() {
        return iterator(this->get_head()->mp_fwd_nodes[0]);
    }
    
    iterator end() {
        return iterator(this->get_tail());
    }

    std::pair<iterator, iterator>
    range(const Key_T* key_1, const Key_T* key_2) {
        std::pair<const typename impl_type::node_type*,
                  const typename impl_type::node_type*> res = find_range(key_1, key_2);

        return make_pair(iterator(res.first), iterator(res.second));
    }

};

}; // end namespace hypocampd

#endif // SKIP_LIST_INCLUDED
