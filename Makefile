# tty_pona - using sitelen pona in terminals
# see: https://github.com/polijan/tty_pona
# Copyright (C) 2023 jan Polijan
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


#----------------------------------------------------------------------
# Your Configuration here:
#----------------------------------------------------------------------

# choose your compiler: either gcc or clang:
CC = gcc
#CC = clang

# uncomment and fill this if you need extra flags, etc.
OPTIONS = 
#OPTIONS = -DTTY_PONA_NO_DLSYM

#======================================================================

CFLAGS=-Wall -Wextra -pedantic -s -O2

.PHONY: all install cleanall

all: lib/tty_pona.so bin/width

lib/tty_pona.so: src/tty_pona.c
	mkdir -p lib
	$(CC) $(CFLAGS) -nostdlib -fPIC -shared $(OPTIONS) -o $@ $< -ldl
	if command -v strip > /dev/null; then strip -s $@; fi
	chmod ugo-x $@

bin/width: src/width.c
	$(CC) $(CFLAGS) -o $@ $<
	if command -v strip > /dev/null; then strip -s $@; fi

install:
	@[ -n "$(INSTALL_DIR)" ] || { echo 'INSTALL_DIR must be defined' >&2; exit 1; }
	mkdir -p "$(INSTALL_DIR)"/bin "$(INSTALL_DIR)"/lib
	cp -i lib/tty_pona.so "$(INSTALL_DIR)"/lib
	cp -i bin/* "$(INSTALL_DIR)"/bin

cleanall:
	rm -f lib/tty_pona.so bin/width
