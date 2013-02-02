/*
 * w_noncopyable.h
 * copy form boost/noncopyable
 */

#ifndef W_NONCOPYABLE_H_
#define W_NONCOPYABLE_H_  

namespace wlib {
   namespace noncopyable_ {
      class noncopyable {
      protected:
         noncopyable() {}
         ~noncopyable() {}
      private:
         noncopyable(const noncopyable&);
         const noncopyable& operator=(const noncopyable&);
      };
   }

   typedef noncopyable_::noncopyable noncopyable;
}

#endif // W_NONCOPYABLE_H_
