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

   inline w_logger::w_logger(void) : 
   level_(0x0f), f_(0), stoped_(false), std_out_(true), encrypted_(false) {
   }

   inline w_logger::~w_logger(void) {
      if (f_) {
         fclose(f_);
      }
   }

   inline int w_logger::log(int level, const char* fmt, ...) {
      std::lock_guard<std::mutex> lock(this->mutex_);

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

      sprintf(buf, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d==>", 
         tt->tm_year+1900, tt->tm_mon+1, tt->tm_mday,
         tt->tm_hour, tt->tm_min, tt->tm_sec);

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

      if (!this->f_) {
         return 0;
      }

      fwrite(buf,meslen,1,this->f_);
      fflush(this->f_);
      return 0;
   }

   inline void w_logger::std_out(bool onoff) {
      std::lock_guard<std::mutex> lock(this->mutex_);
      this->std_out_ = onoff;
   }

   inline int w_logger::set_output_file(const char* fpath) {
      std::lock_guard<std::mutex> lock(this->mutex_);
      //this->fpath_ = fpath;
      this->f_ = ::fopen(fpath, "a+");
      return 0;
   }

   inline void w_logger::set_debug_level(int level) {
      std::lock_guard<std::mutex> lock(this->mutex_);
      this->level_ = level;
   }

   inline void w_logger::set_encrypt(bool encrypt) {
      std::lock_guard<std::mutex> lock(this->mutex_);
      this->encrypted_ = encrypt;
   }

   inline int w_logger::encrypted_log(char* buf, int buflen) {
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

   inline void w_logger::start_log() {
      std::lock_guard<std::mutex> lock(this->mutex_);
      this->stoped_ = false;
      if (this->f_ && this->fpath_.length() > 0) {
         FILE* fp = ::fopen(this->fpath_.c_str(), "ab+");
         if(!fp) {
            return;
         }
         this->f_ = fp;
      }
   }

   inline void w_logger::stop_log() {
      std::lock_guard<std::mutex> lock(this->mutex_);
      this->stoped_ = true;
   }

} //namespace wlib

