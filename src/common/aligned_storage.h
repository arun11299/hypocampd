#ifndef HYPOCAMPD_ALIGNED_STORAGE_H
#define HYPOCAMPD_ALIGNED_STORAGE_H

#include <type_traits>
#include <utility>

namespace hypocampd {

    template <typename TypeT>
    class AlStore {
    public:
	template <typename... Args>
	AlStore(Args&&... args) {
	    new (&m_aldata) TypeT(std::forward<Args>(args)...);
	}

	~AlStore() {
	    reinterpret_cast<TypeT*>(&m_aldata)->~TypeT();
	}

        AlStore(AlStore&& other) {
            this->m_aldata = other.m_aldata;     
        }

	void operator= (const AlStore<TypeT>& other) = delete;
  
	// Get the primitive object  
	inline
	TypeT& object() noexcept {
	    return this->operator*();
	}

	inline 
	const TypeT& object() const noexcept {
	    return this->operator*();
	}

	// Dereferencing operators
	inline
	TypeT& operator* () noexcept {
	    return *reinterpret_cast<TypeT*>(&m_aldata);
	}

	inline
	const TypeT& operator*() const noexcept {
	    return *reinterpret_cast<const TypeT*>(&m_aldata);
	}

	inline
	TypeT& operator->() noexcept {
	    return *reinterpret_cast<TypeT*>(&m_aldata);
	}

	inline
	const TypeT& operator->() const noexcept {
	    return *reinterpret_cast<const TypeT*>(&m_aldata);
	}

    private:
	typename std::aligned_storage<sizeof(TypeT), 
                                      alignof(TypeT)>::type m_aldata;
    };


}; // END namespace hypocampd

#endif
