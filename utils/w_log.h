/*
 * w_log.h
 */

#ifndef W_LOG_H_
#define W_LOG_H_

#include "w_singleton.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <mutex>

namespace wlib {

//#define W_NLOGGING
#define W_NTRACE

static void empty_func(int, const char*, ...) {}
   
#if defined (W_NLOGGING)
#  define w_dbg  empty_func
#else
#  define w_dbg Logger::instance()->log  // for remove (())
#endif // (W_NLOGGING)

#define w_trace(x) W_TRACE(x)


#if defined (W_NTRACE)
# define W_TRACE(X) do{}while(0)
#else // (W_NTRACE)
#  define W_TRACE(X)  w_tracer ____ ((__FILE__), (X), __LINE__)
#endif // (W_NTRACE)

enum Log_Level {
	FATAL = 0x01,	/* 系统或调试程序时出现最致命错误的日志级别*/
	CRIT  = 0x02,	/* 系统或调试程序时出现标准错误的日志级别*/
	WARN  = 0x04,	/* 系统或调试程序时出现警告错误的日志级别*/
	INFO  = 0x08,	/* 系统或调试程序时的信息日志级别*/
	DEBUG = 0x10	/* 调试程序时的调试日志级别*/
};

class w_logger;

typedef w_singleton<w_logger> Logger;

class w_logger {
public:
	w_logger(void);
	~w_logger(void);
public:
	inline int log(int level, const char* fmt, ...);
   inline void start_log();
   inline void stop_log();
	inline void std_out(bool onoff);
	inline int set_output_file(const char* fpath);
   inline void set_debug_level(int level);
   inline void set_encrypt(bool encrypted);
private:
   inline int encrypted_log(char* buf, int buflen);
private:
	int level_;
   bool stoped_;
	FILE* f_;
	bool std_out_;
   std::mutex mutex_;
	std::string fpath_;
   bool encrypted_;
};



struct w_tracer {
	w_tracer(const char* fname, const char* mname, int linenum) 
		: f_(fname), m_(mname), l_(linenum) {
		w_dbg(INFO, "<+++ %s +++> in the %s at the %d line.\n", 
			mname, fname, linenum);
	}
	~w_tracer(void) {
		w_dbg(INFO, "<--- %s ---> in the %s at the %d line.\n", 
			m_.c_str(), f_.c_str(), l_);
	}
private:
	std::string f_;
	std::string m_;
	unsigned l_;
};

} //namespace wlib

#include "w_log.inl"

#endif // W_LOG_H_
