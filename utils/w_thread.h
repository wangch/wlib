/*
 * w_thread.h
 */

#ifndef W_THREAD_H_
#define W_THREAD_H_

#ifdef _WINDOWS
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

namespace wlib
{

#ifdef _WINDOWS
typedef HANDLE  w_thread_t;
typedef void* (*thr_f)(void*);
typedef void (*thr_wf)(void*);
inline int w_thread_create(w_thread_t* r, bool j, thr_f f, void* p)
{
	_beginthread((thr_wf)f, 0, p);
	return 0;
}
inline w_thread_t w_thread_self()
{
	return GetCurrentThread();
}
#else // _WINDOWS
typedef pthread_t w_thread_t;
inline int w_thread_join(w_thread_t thr)
{
	void* p = 0;
	return pthread_join(thr, &p);
}
inline int w_thread_create(w_thread_t* r, bool j, void* (*f)(void*), void* p)
{
	int re = pthread_create(r, NULL, f, p);
	if (!re && j) {
		return w_thread_join(*r);
	}
	return re;
}
inline w_thread_t w_thread_self()
{
	return pthread_self();
}
#endif // _WINDOWS

template<class T>
class w_thread
{
	bool join_able_;
	w_thread_t thr_;

	static void* thr_func(void* p)
	{
		T* t = (T*)p;
		t->thr_exec();
		return NULL;
	}
public:
	w_thread() : join_able_(true) {}
	~w_thread() {}

	int run(T* t, bool joinable = true)
	{
		join_able_ = joinable;
		return w_thread_create(&thr_, joinable, w_thread::thr_func, t); 
	}
	/*
	int join()
	{
		if (join_able_)
			return w_thread_join(thr_);
		return 0;
	}
	*/
};

/*
class w_thread_pool
{
	template<typename template<class T> Thread<T> >
};
*/

} // namespace wlib

#endif // W_THREAD_H_
