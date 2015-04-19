#ifndef HYPOTHALD_REFERENCE_COUNT_H
#define HYPOTHALD_REFERENCE_COUNT_H

#include <atomic>
#include <boost/noncopyable.hpp>
#include <boost/intrusive_ptr.hpp>

namespace hypocampd {

    // Fwd decl
    template <typename T>
    class ReferenceCounted;

    template <typename T>
    void intrusive_ptr_add_ref(ReferenceCounted<T>* rc);

    template <typename T>
    void intrusive_ptr_release(ReferenceCounted<T>* rc);
   
    template <typename TypeT>
    class ReferenceCounted : boost::noncopyable {
    public:
        ReferenceCounted():m_cnt(0) {}

        template <typename T>
        friend void intrusive_ptr_add_ref(ReferenceCounted<T>* rc);

        template <typename T>
        friend void intrusive_ptr_release(ReferenceCounted<T>* rc);

        virtual ~ReferenceCounted() {return;}

    private:
        TypeT m_cnt; 
    };

    template <typename T>
    void intrusive_ptr_add_ref(ReferenceCounted<T>* rc) {
        ++(rc->m_cnt);
    }

    template <typename T>
    void intrusive_ptr_release(ReferenceCounted<T>* rc) {
        if (--(rc->m_cnt) == 0) {
            delete rc; 
        }
    }

    // Aliases for shorthand usage
    using AtomicRefCounter = ReferenceCounted<std::atomic_int>;
    using SimpleRefCounter = ReferenceCounted<int>;

}; // END namespace hypocampd

#endif // HYPOTHALD_REFERENCE_COUNT_H
