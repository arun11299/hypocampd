#ifndef HYPOCAMPD_SIMPLE_STRING_H
#define HYPOCAMPD_SIMPLE_STRING_H

#include <cstring>
#include <cassert>
#include <memory>
#include <utility>
#include <algorithm>

namespace hypocampd {

static const short string_hdr_len = sizeof(uint32_t) +  // len_
                                    sizeof(uint32_t) +  // free_
                                    sizeof(char*);      // buf_

static const double GROWTH_FACTOR = 1.5;

static const short SSO_SIZE = 15;

// TODO Provide compilation flags to turn off SSO


/* 
 * sstring : simple string class (not so simple anymore!)
 * Idea was to have a lightweight string class with 
 * Short string optimization(SSO). g++ used to do SSO, but
 * more recent compilers may drop support for it.
 * 
 * NOTE: For SSO, string size to be stored is limited to
 *       15 bytes and 1 byte for null delimiter.
 * If using SSO, better avoid append operations that 
 * may result in string length more than 15 bytes for
 * performance reasons.
 * Assignment operators for std::move are as costly as copying
 * in case of SSO.
 */
class sstring {
public:
    // Default constructor
    // Creates a string_rep instance with 0
    // bytes allocated for data buffer
    sstring() {
        init_new_srep_struct(0);
    }

    // Constructor 2
    // Takes string literal as first argument and
    // a bool to tell whether client wants to do
    // SSO or not. sstring by default does SSO.
    sstring(const char* str, bool do_sso = true) {
        assert(str);
        uint32_t len = strlen(str);
        
        /*
         * String representation
            --------------------------------------
            |len    |free  |buf_  | ...data...
            --------------------------------------
                            |      ^
                            |      |
                            --------
            This gives better cache locality.
        
         */
        init_new_srep_struct(len, do_sso);

        if (m_sso) sstring_copy(str, sso_buf_, len);
        else       sstring_copy(str, mp_srep->buf_, len);
    }

    // Constructor just taking the size of the buffer
    // which needs to be allocated.
    // The buffer is default initialized 
    sstring(size_t size, bool do_sso = true) {
        init_new_srep_struct(size, do_sso);
    }

    // Copy c'tor
    sstring(const sstring& other) {
        init_new_srep_struct(other.mp_srep->len_);
        if (m_sso) 
            sstring_copy(other.mp_srep->buf_, sso_buf_, 
                         other.mp_srep->len_);
        else
            sstring_copy(other.mp_srep->buf_, mp_srep->buf_, 
                         other.mp_srep->len_);
    }

    // Move c'tor
    sstring(sstring&& other) {
        if (other.m_sso) {
            sstring_copy(other.sso_buf_, sso_buf_, other.length());
            this->m_sso = true;
            return;
        }
        mp_srep = std::move(other.mp_srep);
        other.mp_srep.release();
    }

    // Assignment operator
    // If the 'other' object has sso set and 
    // this object do not, then also the small
    // string will be stored in stack for 'this'
    // and m_sso flag would be set.
    const sstring& operator=(const sstring& other);

    // Assignment operator number 2 for string literal
    // Not required !! This is the beauty of C++ which happens behind
    // the scenes.
    // The constructor for sstring is not marked as explicit
    // and it takes single argument as string literal. Hence
    // when we assign a string literal to a constructed sstring object
    // it automatically calls the above assignment operator.

    // Move assignment operator
    const sstring& operator=(sstring&& other) {
        if (other.m_sso) {
            this->clear();
            m_sso = true;
            sstring_copy(other.sso_buf_, sso_buf_, other.length());
            return *this;
        }
        mp_srep.reset(other.mp_srep.get());
        other.mp_srep.release();
        m_sso = false;
        return *this;
    }

   
    // Returns the pointer to the start of buffer 
    const char* c_str() const noexcept {
        if (m_sso) return sso_buf_;
        else       return mp_srep->buf_;
    }

    // Returns the free space in buffer
    size_t free_space() const noexcept {
        if (m_sso) return SSO_SIZE - length();
        else       return mp_srep->free_;
    }

    // Returns the length of data in buffer
    size_t length() const noexcept {
        if (m_sso) return strlen(sso_buf_);
        else       return mp_srep->len_;
    }

    // Returns the total capacity
    size_t capacity() const noexcept {
        return length() + free_space();
    }

    // Clear the buffer area
    void clear() noexcept {
        if (m_sso) {
            memset(sso_buf_, 0, SSO_SIZE + 1);
            return;
        }

        memset(mp_srep->buf_, '\0', capacity());
        mp_srep->free_ = capacity();
        mp_srep->len_  = 0;
        return;
    }

    // Clone the string container
    sstring clone() {
        return sstring(*this); 
    }

    void __dump__() const {
        if (m_sso) {
            std::cout << "Short string optimized" << std::endl;
            return;
        }
        std::cout << "Length = " << mp_srep->len_ << std::endl;
        std::cout << "Free   = " << mp_srep->free_ << std::endl;
    }

    //------------------------------------------
    // Implementation of some string algorithms |
    //------------------------------------------
  

    // This is a fast implementation of trim function
    // which trades memory for faster trim operation
    // by just moving pointers to the correct place.
    // The useless characters are still present in the 
    // buffer 
    const sstring& trim(const char* tset = " ");

   
    // This is the memory efficient version of the string
    // This function should be used only when there is a chance
    // of large data will be trimmed off. 
    const sstring& trim_mem_eff(const char* tset  = " ");


    // Store only the substring found in the
    // provided range.
    void substr(int s_pos, int e_pos);
       

    // Appends the passed str to the buffer
    // If space in buffer is less than what is required
    // the buffer will be expanded to meet the requirement;
    void append(const char* str);

    // Comparison operator
    // Would return true if both sstring object matches
    // and false if they do not match
    bool operator==(const sstring& other) const noexcept;

    // Shrink the buffer to exact memory
    // Use when you have to release extra memory
    // after using non memory efficient but fast 
    // trim and substr methods
    void shrink_to_fit() ;

protected:

    void sstring_copy(const char* source, char* dest, uint32_t len, 
                      bool update_req = true) {
        if (len >= 4) do_fast_copy(source, dest, len);
        else std::copy_n(source, len, dest);

        // TODO make this if as unlikely
        if (mp_srep != nullptr && update_req) {
            mp_srep->len_  += len;
            mp_srep->free_ -= len;
        } 

        return;
    }

    void do_fast_copy(const char* source, char* dest, uint32_t len) {
        while (len >= 4) {
            *(reinterpret_cast<int*>(dest)) = 
                                    *(reinterpret_cast<const int*>(source));
            len -= 4;
            dest += 4; source += 4;
        }

        if (len) std::copy_n(source, len, dest);
    }

private: // Private Member Functions

    void init_new_srep_struct(uint32_t length, bool do_sso = true) {
        if (do_sso && length <= SSO_SIZE) {
            m_sso = true; 
            return;
        }
        char* buf = new char[sizeof(string_rep) + length + 1]();
        mp_srep.reset(reinterpret_cast<string_rep*>(buf));    
        mp_srep->buf_ = buf + string_hdr_len;
        mp_srep->len_ = 0;
        mp_srep->free_ = length;
        return;
    }


    char* __expand__(uint32_t new_size) {
        // this function will always be called when there is 
        // real need of expansion. Therefore, it is not required
        // to assert if the expansion is really required or not
        char* buf = new char[sizeof(string_rep) + new_size + 1]();
        string_rep* str = reinterpret_cast<string_rep*>(buf);

        str->buf_  = buf + string_hdr_len;
        sstring_copy(mp_srep->buf_, str->buf_, mp_srep->len_, false);
        
        str->len_  = this->mp_srep->len_;
        str->free_ = new_size - str->len_;

        return buf;
    }

private: // Private Data Members
    struct string_rep {
        // The MSB bit of length field is used
        // for determining whether SSO technique 
        // is used or not.
        // If MSB is set to 1, that means SSO
        // is used. Else, SSO is not used.
        uint32_t len_  = 0;
        uint32_t free_ = 0;
        char* buf_ = nullptr;  
    };

    // For short string optimization
    char sso_buf_[SSO_SIZE + 1] = {0,};
    bool m_sso = false;

    struct StringDeleter {
        void operator()(string_rep* resource) const {
            char* buf = reinterpret_cast<char*>(resource);
            delete[] buf;
        }
    };

    std::unique_ptr<string_rep, StringDeleter> mp_srep = nullptr;
};


// ===================================================================
// Member function implementation

inline const sstring& 
sstring::operator=(const sstring& other) 
{
    if (other.m_sso) {
        mp_srep.release();
        m_sso = true;
        clear();
        sstring_copy(other.sso_buf_, sso_buf_, other.length());
        return *this;
    }
    // Cannot use init_new_srep_struct here. If used this
    // function would not be exception neutral 

    char* buf = new char[sizeof(string_rep) + other.mp_srep->len_ + 1](); 
    sstring_copy(other.mp_srep->buf_, buf + sizeof(string_rep), 
                 other.mp_srep->len_, false); 

    mp_srep.reset(reinterpret_cast<string_rep*>(buf));

    mp_srep->len_  = other.mp_srep->len_;
    mp_srep->free_ = other.mp_srep->free_;
    mp_srep->buf_  = buf + string_hdr_len;

    return *this;
}

// ===================================================================

inline bool
sstring::operator==(const sstring& other) const noexcept
{
    if (length() != other.length()) return false;
  
    const char* first  = nullptr; const char* second = nullptr;

    if (m_sso) first = sso_buf_;
    if (other.m_sso) second = other.sso_buf_;

    if (!first)  first  = mp_srep->buf_;
    if (!second) second = other.mp_srep->buf_;

    uint32_t len = length();

    while (len >= 4) {
        if ( *(reinterpret_cast<int*>((char*)first)) != 
             *(reinterpret_cast<int*>((char*)second))
           ) {
            return false;
        }
        len -= 4;
        first += 4; second += 4;
    }

    if (len) {
        return memcmp(first, second, len) == 0;
    }

    return true;
}

// ===================================================================

inline void 
sstring::substr(int s_pos, int e_pos) 
{

    if (length() == 0) return;

    char* s = nullptr; char* e = nullptr;

    if (m_sso) {
        s = sso_buf_;
        e = &sso_buf_[length()]; 
    } else {
        s = mp_srep->buf_;
        e = mp_srep->buf_ + length();
    }

    if (s_pos < 0) s_pos = 0;
    if (e_pos < 0) e_pos = 0;

    s += s_pos;
    mp_srep->len_  -= s_pos;
    e_pos = e - (s + e_pos) + 1;
        
    char* tmp_e = e;

    while (e_pos > 0) { *e-- = '\0'; e_pos--; }

    mp_srep->buf_ = s;
    mp_srep->free_ += (tmp_e - e);
    mp_srep->len_  -= (tmp_e - e);

    return;
}

// ===================================================================

const sstring& 
sstring::trim_mem_eff(const char* tset) 
{
    char* c = nullptr; char* e = nullptr;

    if (m_sso) {
        c = sso_buf_;
        e = &sso_buf_[length()]; 
    } else {
        c = mp_srep->buf_;
        e = mp_srep->buf_ + length();
    }

    while (c <= e && strchr(tset, *c)) {
        c++;
    }
    while (e > c && strchr(tset, *e)) {
        e--;
    }

    char* buf = new char[sizeof(string_rep) + (e - c + 1) + 1];
    sstring_copy(c, buf + string_hdr_len, (e - c + 1));
    mp_srep.release();
    mp_srep.reset(reinterpret_cast<string_rep*>(buf));
    mp_srep->buf_ = buf + string_hdr_len;

    mp_srep->len_ = (e - c + 1);
    mp_srep->free_ = 0;

    return *this;
}

// ===================================================================

inline const sstring& 
sstring::trim(const char* tset) 
{
    char* c = nullptr; char* e = nullptr;

    if (m_sso) {
        c = sso_buf_;
        e = &sso_buf_[length()]; 
    } else {
        c = mp_srep->buf_;
        e = mp_srep->buf_ + length();
    }

    while (c <= e && strchr(tset, *c)) {
        c++;
    }
    mp_srep->len_ -= (c - mp_srep->buf_);
    mp_srep->buf_ = c;

    char* tmp_e = e;
    while (e > c && strchr(tset, *e)) {
        *e = '\0';
        e--;
    }
    mp_srep->len_  -= (tmp_e - e) - 1;
    mp_srep->free_ += (tmp_e - e);

    return *this;
}

// ===================================================================

inline void 
sstring::append(const char* str) 
{
    uint32_t slen = strlen(str);

    if ( slen > free_space() ) { 
        uint32_t new_size = (length() + slen) * GROWTH_FACTOR;
        if (m_sso) {
            short len = length();
            init_new_srep_struct(new_size); 
            sstring_copy(sso_buf_, mp_srep->buf_, len);
            m_sso = false;
        } else {
            char* buf = __expand__(new_size);
            mp_srep.release();
            mp_srep.reset(reinterpret_cast<string_rep*>(buf));
        }
    }

    if (!m_sso) sstring_copy(str, mp_srep->buf_ + length(), slen);
    else        sstring_copy(str, sso_buf_ + length(), slen);

    return;
}

// ===================================================================

inline void
sstring::shrink_to_fit() 
{
    if (m_sso) return;

    char* data = mp_srep->buf_;

    // If strlen of actual buffer is equal to the 
    // reported length, that means we have not
    // used the optimized version of trim and substr
    // on this object till now
    if (strlen(data) == capacity()) return;

    uint32_t clen = length();
    char* new_buf = new char[sizeof(string_rep) + length() + 1]();
    sstring_copy(data, new_buf + string_hdr_len, length(), false);
    mp_srep.reset(reinterpret_cast<string_rep*>(new_buf));

    mp_srep->buf_  = new_buf + string_hdr_len;
    mp_srep->free_ = 0;
    mp_srep->len_  = clen;

    return;
}

}; // End namespace hypocampd

#endif
