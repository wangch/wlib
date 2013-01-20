/*
 * w_lock.inl
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <iostream>

#ifdef _WIN32
#pragma warning(disable:4996)
#endif

namespace wlib
{

	template<class Lock>
		inline w_logger<Lock>::w_logger(void) : 
			level_(0x0f), f_(0), stoped_(false), std_out_(true), encrypted_(false) {
	}

	template<class Lock>
		inline w_logger<Lock>::~w_logger(void) {
         if (f_) {
			   fclose(f_);
         }
		}

	template<class Lock>
		inline int w_logger<Lock>::log(int level, const char* fmt, ...) {
			Lock lock(this->mutex_);

			if(this->stoped_)
				return 0;

			if(this->level_ < level){
				return 1; // not output; 
			}

			va_list args;
			va_start(args, fmt);

			char buf[2048] = {0};
			time_t t = time(0);
			struct tm* tt = ::localtime(&t);

			w_thread_t thr = w_thread_self();
			sprintf(buf, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d %p==>", 
					tt->tm_year+1900, tt->tm_mon+1, tt->tm_mday,
					tt->tm_hour, tt->tm_min, tt->tm_sec, thr);

			size_t len = strlen(buf);
			vsnprintf(buf+len,(sizeof(buf) - len - 4), fmt, args);
			va_end(args);

			strcat(buf, "\n");
			if(this->std_out_)
				std::cout << buf;
			int meslen = (int)strlen(buf);
			if(this->encrypted_){
				this->encrypted_log(buf, meslen);
			}
			if (this->fpath_.length() == 0) {
				return 0;
			}

			fwrite(buf,meslen,1,this->f_);
			fflush(this->f_);
			return 0;
		}

	template<class Lock>
		inline void w_logger<Lock>::std_out(bool onoff) {
			Lock lock(this->mutex_);
			this->std_out_ = onoff;
		}

	template<class Lock>
		inline int w_logger<Lock>::set_output_file(const char* fpath) {
			Lock lock(this->mutex_);
			this->fpath_ = fpath;
			return 0;
		}

	template<class Lock>
		inline void w_logger<Lock>::set_debug_level(int level) {
			Lock lock(this->mutex_);
			this->level_ = level;
		}

	template<class Lock>
		inline void w_logger<Lock>::set_encrypt(bool encrypt) {
			Lock lock(this->mutex_);
			this->encrypted_ = encrypt;
		}

	template<class Lock>
		inline int w_logger<Lock>::encrypted_log(char* buf, int buflen) {
			if(!buf || buflen <= 0)
				return -1;

			char* p = buf;
			while(buflen){
				*p = ~(*p);   
				++p;
				buflen --;
			}
			return 0;
		}

	template<class Lock>
		inline void w_logger<Lock>::start_log() {
         Lock lock(this->mutex_);
         this->stoped_ = false;
         if (this->f_ && this->fpath_.length() > 0) {
            FILE* fp = ::fopen(this->fpath_.c_str(), "ab+");
            if(!fp) {
               return -1;
            }
            this->f_ = fp;
         }
      }

	template<class Lock>
		inline void w_logger<Lock>::stop_log() {
			Lock lock(this->mutex_);
			this->stoped_ = true;
		}

} //namespace wlib

