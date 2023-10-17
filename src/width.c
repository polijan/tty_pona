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

#define _XOPEN_SOURCE
#include <wchar.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
   if (argc != 2) {
      fprintf(stderr, "Usage: %s <string>\n", argv[0]);
      exit(EXIT_FAILURE);
   }

   /* convert argv[1] to a wchar string (in variable wcs): */

   /* we can allocate wchar based on number of bytes,
      this is simpler but we may be over allocating:
        | size_t n = strlen(argv[1]);
      or we can use do the conversion to wchar string
      to define the size:
        | size_t n = mbstowcs(NULL, argv[1], 0);
        |  if (mbstowcs(wcs, argv[1], n+1) == (size_t) -1) {
        |     perror("mbstowcs");
        |     exit(EXIT_FAILURE);
        |  }
   */
   size_t n = strlen(argv[1]);

   wchar_t *wcs = malloc((n+1) * sizeof(wchar_t));
   if (!wcs) {
      perror("malloc");
      exit(EXIT_FAILURE);
   }
   if (setlocale(LC_ALL, "") == NULL) {
      perror("setlocale");
      exit(EXIT_FAILURE);
   }
   if (mbstowcs(wcs, argv[1], n+1) == (size_t) -1) {
      perror("mbstowcs");
      exit(EXIT_FAILURE);
   }

   /* we can finally use wcswidth: */
   int w = wcswidth(wcs, n);
   free(wcs);
   if (w < 0)
      return 1;
   printf("%d\n", w);
   return 0;
}
