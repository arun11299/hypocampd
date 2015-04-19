#ifndef HYPOCAMPD_SPIN_LOCK_H
#define HYPOCAMPD_SPIN_LOCK

#include <atomic>
#include <utility>

namespace hypocampd {

    class SpinLock {
    public:
	SpinLock() {
	    m_lock.clear();
	}

	~SpinLock() {
	    m_lock.clear(std::memory_order_acquire);
	}

	inline 
	void lock() {
	    while (m_lock.test_and_set(std::memory_order_acquire));
	}

	inline 
	void unlock() {
	    m_lock.clear(std::memory_order_release);
	}

	inline 
	bool try_lock() {
	    return !m_lock.test_and_set(std::memory_order_acquire);
	}

    private:
	std::atomic_flag m_lock;
    };

}; // END namespace hypocampd

#endif
