/*
 * w_singleton.h
 */

#ifndef W_SINGLETON_H_
#define W_SINGLETON_H_

#include <mutex>
#include <stdlib.h>

namespace wlib {

template<typename TYPE>
class w_singleton {
protected:
	w_singleton(void){}
	virtual ~w_singleton(void){}
	w_singleton(const w_singleton&);
	w_singleton& operator=(const w_singleton&);

public:
	static TYPE* instance(void) {
		if(instance_ == 0) {
         std::lock_guard<std::mutex> lock(mutex_);
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
   static std::mutex mutex_;
};

template<typename TYPE>
TYPE* w_singleton<TYPE>::instance_ = 0;

template<typename TYPE>
std::mutex w_singleton<TYPE>::mutex_;

} //namespace wlib

#endif // W_SINGLETON_H_
