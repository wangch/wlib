/*
 * copy form boost/noncopyable
 *
 * wangch
 */

#ifndef NBL_NONCOPYABLE_H
#define NBL_NONCOPYABLE_H  

namespace nbcl
{
   namespace noncopyable_
   {
      class noncopyable
      {
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

#endif //NBL_NONCOPYABLE_H
