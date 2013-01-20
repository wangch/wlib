/*
 * w_lock.h
 */


#ifndef W_LOCK_H_
#define W_LOCK_H_

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else // WINDOWS
#include <pthread.h>
#endif
#include "w_noncopyable.h"

namespace wlib {

#ifdef WINDOWS
   class mutex {
private:

    CRITICAL_SECTION cs_;

    mutex(mutex const &);
    mutex & operator=(mutex const &);

public:

    mutex() {
        InitializeCriticalSection(&cs_);
    }

    ~mutex() {
        DeleteCriticalSection(&cs_);
    }

    void lock() {
        EnterCriticalSection(&cs_);
    }
// TryEnterCriticalSection only exists on Windows NT 4.0 and later
#if (defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0400))
    bool try_lock() {
        return TryEnterCriticalSection(&cs_) != 0;
    }
#else
    bool try_lock() {
        return false;
    }
#endif
    void unlock() {
        LeaveCriticalSection(&cs_);
    }
};

#else // WINDOWS

class mutex {
private:

    pthread_mutex_t m_;

    mutex(mutex const &);
    mutex & operator=(mutex const &);

public:

    mutex() {
        pthread_mutex_init(&m_, 0);
    }

    ~mutex() {
        pthread_mutex_destroy(&m_);
    }

    void lock() {
        pthread_mutex_lock(&m_);
    }

    bool try_lock() {
        return pthread_mutex_trylock(&m_) == 0;
    }

    void unlock() {
        pthread_mutex_unlock(&m_);
    }
};

#endif //WINDOWS


struct null_lock : public wlib::noncopyable {
   null_lock(mutex& m) {}
};

class w_lock : public wlib::noncopyable {
public:
    w_lock(mutex& m): mutex_(m) {
        mutex_.lock();
    }
    ~w_lock() {
        mutex_.unlock();
    }
private:
    mutex &mutex_;
};

typedef w_lock mt_lock;

struct w_cond {
	mutex& m1_;
	mutex m2_;

	void wait(mutex& m) {
	}

};

} // namespace wlib

#endif // W_LOCK_H_
