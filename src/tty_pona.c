/* tty_pona - using sitelen pona in terminals
 * see: https://github.com/polijan/tty_pona
 * Copyright (c) 2023 jan Polijan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* we only need `dlsym` and `wcwidth`'s signature, which are POSIX-1.2001 */
#define _POSIX_SOURCE    1
#define _POSIX_C_SOURCE  200112L
#define _XOPEN_SOURCE    600
#define _GNU_SOURCE

#include <wchar.h>
/* (`wcwidth` returns the number of columns needed to represent a character) */

/* minimal `wcwidth`: everything monospace (except ASCII controls) */
static int wcwidth_minimal(wchar_t c)
{
   if (c == 0)
      return 0;
   else if ((c <= 31) || (c == 127))
      return -1;
   else
      return 1;
}

/* `wcwidth`: implementation via calling the original C library function */
#ifndef TTY_PONA_NO_DLSYM
#include <dlfcn.h>
static int wcwidth_libc(wchar_t c)
{
   static int (*wcwidth_)(wchar_t) = 0;

   if (!wcwidth_) {
      *(void**)(&wcwidth_) = dlsym(RTLD_NEXT, "wcwidth");
      /* ^-- same as: wcwidth_libc = dlsym(RTLD_NEXT, "wcwidth");
       * In ISO C, casting between function pointers and void* is undefined
       * behaviour. Most architectures supports such casting of course. And
       * indeed the 2013 Technical Corrigendum 1 to POSIX.1-2008 states that
       * a conforming POSIX implementations should support such. */

      if (!wcwidth_) { /*<-- error would be weird (but you never know)*/
         return wcwidth_minimal(c);
      }
   }
   return (*wcwidth_)(c);
}
#endif

/* "hijacked" `wcwidth` for sitelen pona */
int wcwidth(wchar_t c)
{
   /* sitelen pona UCSUR character: we say it's a double-width character, except
    * for some controls (combining, joining, and "long" glyphs combo [but we can
    * still print `START OF LONG PI`] which are best hidden.
    */
   if ((c >= 0xF1900ul) && (c <= 0xF19FFul)) {
      if ((c >= 0xF1992ul) && (c <= 0xF199Bul) && (c != 0xF1993ul)) {
         return -1;
      }
      return 2;
   }

   /* non sitelen pona characters */
#ifdef TTY_PONA_NO_DLSYM
   return wcwidth_minimal(c);
#else
   return wcwidth_libc(c);
#endif
}

/* "hijacked" `wcswidth` (implemented using with `wcwidth`) */
int wcswidth(const wchar_t *s, size_t n)
{
   int wc, w = 0;

   for (;*s && (n-- > 0); s++) {
      if ((wc = wcwidth(*s)) < 0) {
         return -1;
      } else {
         w += wc;
      }
   }
   return w;
}
