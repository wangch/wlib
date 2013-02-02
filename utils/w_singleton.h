/*
 * w_singleton.h
 */

#ifndef W_SINGLETON_H_
#define W_SINGLETON_H_

#include "w_lock.h"
#include <stdlib.h>

namespace wlib {

template<typename TYPE, typename LOCK = null_lock>
class w_singleton {
   typedef LOCK scoped_lock;
protected:
	w_singleton(void){}
	virtual ~w_singleton(void){}
	w_singleton(const w_singleton&);
	w_singleton& operator=(const w_singleton&);

public:
	static TYPE* instance(void) {
		if(instance_ == 0) {
			scoped_lock lock(mutex_);
			if(instance_ == 0)
				instance_ = new TYPE();

			// for in atexit call ~TYPE()
			//::atexit(destroy);
		}

		return instance_;
	}

	static void destroy(void) {
		delete instance_;
	}

private:
	static TYPE* instance_;
	static w_mutex mutex_;
};

template<typename TYPE, typename LOCK>
TYPE* w_singleton<TYPE, LOCK>::instance_ = 0;

template<typename TYPE, typename LOCK>
w_mutex w_singleton<TYPE, LOCK>::mutex_;

} //namespace wlib

#endif // W_SINGLETON_H_
