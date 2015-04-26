#ifndef HYPOCAMPD_ABSTRACT_OBJECT_H
#define HYPOCAMPD_ABSTRACT_OBJECT_H

#include <memory>

namespace hypocampd {

/*
 * AObject: Abstract Object
 * It is a basic type erasure technique for having
 * a common object notation for all different types
 * of data structures.
 * NOTE :- This is a move only type class.
 */
class AObject {
public:
    AObject() {}

    template <typename OrigType>
    AObject(const OrigType* base_obj) : 
                    mp_holder(new Holder<OrigType>(base_obj))
    {}

    // Move constructor
    AObject(AObject&& other) noexcept {
        mp_holder.release();
        mp_holder = std::move(other.mp_holder);
    }

    // If someone wants to manually destroy
    // the AObject rather than relying on RAII
    void destroy() noexcept {
        mp_holder.release();
    }

    // Copying is not an option
    AObject(const AObject& other) = delete;
    void operator=(AObject other) = delete;

    struct BaseHolder {
        BaseHolder() = 0; 
    };

    // Holder Object
    template <typename OrigType>
    struct Holder: public BaseHolder {
    public:
        Holder(const OrigType* base_obj): 
                        mp_heldobj(std::unique_ptr<OrigType>{base_obj})
        {}

        std::unique_ptr<OrigType> mp_heldobj = nullptr;
    };

    std::unique_ptr<BaseHolder> mp_holder = nullptr;
};



// This is an unsafe cast
template <typename T>
T* aobj_cast(const AObject& o) {
    return static_cast<T*>( 
                static_cast<AObject::Holder<T>*>(o.mp_holder.get())->mp_heldobj
        );
}

}; // End namespace hypocampd
#endif
