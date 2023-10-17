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

#define _POSIX_SOURCE 1
#define _POSIX_C_SOURCE
#define _XOPEN_SOURCE
#define _GNU_SOURCE
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

static void usage(FILE* stream)
{
   fputs("Usage: width <-t|-c> <string>\n", stream);
   fputs("Determine and prints how many columns are needed to print the given string.\n\n", stream);
   fputs("Depending on the given flag, uses one of those two possible method:\n\n", stream);
   fputs("  -c: use wcswidth() from POSIX C\n", stream);
   fputs("      (string must be less than 255 codepoints)\n\n", stream);
   fputs("  -t: figure out by printing the string on the terminal (/dev/tty)\n", stream);
   fputs("      (terminal must be able to print the string on a single line)\n", stream);
   fflush(stream);
}


#define WIDTH_ERROR   (-2)

/* assign terminal's cursor position to row/col. return "false" on error */
static int get_cursor(FILE* tty, int *row, int *col)
{
   /* escape sequence to request cursor position to the terminal */
   fprintf(tty, "\033[6n");
   fflush(tty);

   /* the terminal will answer by "typing" `ESC[<row>;<column>R` */
   return (fscanf(tty, "\033[%d;%dR", row, col) == 2);
}

/* convert the (multibyte) string into a wide character string,
 * and return its lenght using POSIX C's `wcswidth`
 * (return WIDTH_ERROR in case something's wrong)
 */
static int width_c(const char *s)
{
#  define MBS_MAXLEN 255
   wchar_t ws[MBS_MAXLEN + 1];

   if (setlocale(LC_ALL, "") == NULL)
      return WIDTH_ERROR;
   if (mbstowcs(ws, s, MBS_MAXLEN) == (size_t) -1)
      return WIDTH_ERROR;
   return wcswidth(ws, MBS_MAXLEN);
}

static int terminal_width(const char *s)
{
   int ret = WIDTH_ERROR;
   int row_orig, col_orig, row, col;
   struct termios attr, attr_orig;
   FILE* tty = fopen("/dev/tty", "r+");
   if (!tty)  goto out;

   /* disable echo, setup non-canonical mode, and timeout `read` */
   if (tcgetattr(fileno(tty), &attr) != 0)  goto clean_stream;
   attr_orig  = attr;                /* <- save original tty attributes       */
   attr.c_lflag &= ~(ECHO | ICANON); /* <- no echo and set non-canonical mode */
   attr.c_cc[VMIN]  = 0;             /* <-.__ non-blocking `read`             */
   attr.c_cc[VTIME] = 1;             /* <-'   with timeout (1 decisecond)     */
   if (tcsetattr(fileno(tty), TCSAFLUSH, &attr) != 0)  goto clean_stream;

   /* - get original cursor position
    * - send cursor home
    * - write the wide string to the terminal
    * - measure new cursor position (column gives us the width+1 of the string)
    * - restore original cursor position and print the result
    */
   if (!get_cursor(tty, &row_orig, &col_orig))
      goto clean_tty;
   fprintf(tty, "\033[H%s", s);
   if (!get_cursor(tty, &row, &col) || (row != 1) || (col < col_orig))
      goto clean_tty;
   fprintf(tty, "\033[%d;%dH", row_orig, col_orig);
   fflush(tty);
   if (ferror(tty))
      goto clean_tty;
   ret = col - 1;

clean_tty:
   /* restore tty's original settings */
   tcsetattr(fileno(tty), TCSAFLUSH, &attr_orig);
clean_stream:
   fclose(tty);
out:
   return ret;
}


int main(int argc, char *argv[])
{
   if ((argc == 2) && ((strcmp(argv[1], "-h") == 0)    ||
                       (strcmp(argv[1], "--help") ==0) ))
      { usage(stdout); return EXIT_SUCCESS; }

   if ((argc != 3) || (strcmp(argv[1], "-t") && strcmp(argv[1], "-c")))
      { usage(stderr); return EXIT_FAILURE; }

   int w = (argv[1][1] == 'c')?  width_c(argv[2]) : terminal_width(argv[2]);
   if (w == WIDTH_ERROR)
      return EXIT_FAILURE;

   printf("%d\n", w);
   return EXIT_SUCCESS;
}
