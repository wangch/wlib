/*
 * w_type.h
 */


#ifndef W_TYPE_H_
#define W_TYPE_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>

namespace wlib {

#if defined(UNICODE)
	typedef std::wstring tstring;
	typedef wchar_t tchar;
#else //(UNICODE)
	typedef std::string tstring;
	typedef char tchar;
#endif //(UNICODE)

typedef char                  int8;
typedef short                 int16;
typedef long                  int32;
typedef long long             int64;
typedef unsigned char         uint8;
typedef unsigned short        uint16;
typedef unsigned long         uint32;
typedef unsigned long long    uint64;
typedef uint8                 byte;

typedef void* HANDLE;


struct w_deleter { // for for_each
   template<class T>
   void operator()(T* p) { delete p; }
};

template<class T>
struct w_deletor {
private:
   T* t_;
public:
   w_deletor(T* t) : t_(t) {}
   ~w_deletor() {
      delete t_;
   }
}

struct w_buffer_deleter {
private:
	char* buf_;
public:
	w_buffer_deleter(char* buf) : buf_(buf) {
	}
	~w_buffer_deleter() {
		delete [] buf_;
	}
};


} // namespace wlib


#endif // W_TYPE_H_
