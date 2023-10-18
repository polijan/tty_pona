# tty_pona

> pali ni li pana e nasin e ken ni:  
> ilo nanpa la, sina ken kepeken sitelen pona lon poki pimeja ilo pi pana nimi.

## TL;DR

This project provides guidance and necessary tools to setup and use ***sitelen pona*** (script for the constructed language [**toki pona**](https://en.wikipedia.org/wiki/Toki_Pona) [[tok](https://iso639-3.sil.org/code/tok)]) as proper **fullwidth** ideograms **in terminal emulators** under Unix-like systems (mostly *Linux*).

![a text editor running in a terminal emulator with sitelen pona properly setup as fullwidth ideograms](https://cdn.discordapp.com/attachments/994960433951801374/1162480696443535505/image.png)

## Prerequisites

### An UCSUR-compatible font

As of today, the **sitelen pona** script is _not_ present in [Unicode](https://en.wikipedia.org/wiki/Unicode). However, since 2021, the [Under-ConScript Unicode Registry](https://kreativekorp.com/ucsur/) (**UCSUR**) provides a [Private Use Areas](https://en.wikipedia.org/wiki/Private_Use_Areas) de-facto encoding for it in the block `U+F1900` - `U+F19FF`. We're going to use that encoding, so you may want to browse through [**the specification**](https://www.kreativekorp.com/ucsur/charts/sitelen.html). 

Although several Truetype or Opentype sitelen pona fonts are now using the UCSUR codepoints, older fonts are not (so beware). Eventually, you'll want to try out various fonts (perhaps from [this list](https://docs.google.com/spreadsheets/d/1xwgTAxwgn4ZAc4DBnHte0cqta1aaxe112Wh1rv9w5Yk/edit#gid=1195574771) or [this repo](https://github.com/lipu-linku/nasin-sitelen)), but to get started I recommend you begin with [Fairfax HD](https://www.kreativekorp.com/software/fonts/fairfaxhd.shtml). Go ahead and **install the font(s)**

* either system-wide, in `/usr/local/share/fonts`
* or in your home directory, in `~/.local/share/fonts`[*](https://wiki.archlinux.org/title/XDG_Base_Directory) (or `~/.fonts` might work too).

> Assuming your system uses fontconfig, to verify your font installation:  
> use the provided `bin/fc-list-pona` script which lists sitelen pona UCSUR fonts recognized by your system.

As an aside, with font(s) properly installed, you may also want to customize various things (such as the *Discord* messaging application) using this [guide](https://github.com/Id405/sitelen-pona-ucsur-guide/). But let's now continue the setup required for "pona"-ifying the terminal...

### An input method

You might postpone this step, but to work productively at the terminal, you will require a way to easily input the sitelen pona UCSUR glyphs. Several fonts let you input latin characters and show sitelen pona as ligatures but that may not work well on the terminal (often quite the contrary). We want true glyphs entered via an input method. For Unix-like systems, there are currently three main options to choose from:

 - a full [sitelen pona keyboard layout](https://www.kreativekorp.com/software/keyboards/sitelenpona/) which can directly enter all the sitelen pona UCSUR glyphs.
 - an `ibus` and `ibus-table` based method where you enter the word in Latin and it completes into the sitelen pona glyph. This is described [here](https://github.com/Id405/sitelen-pona-ucsur-guide/#linux) and it works!
 - a method called "*Wakalito*" ([instruction and definitions direct download](https://cdn.discordapp.com/attachments/316066233755631616/1061846136907890778/nasin_sitelen_Wakalito_7.3.2.zip)), based on the free software `espanso`, where the left half of the keyboard represents shapes which you input to create sitelen pona glyphs.

(Some of those methods might require a few adjustements to the configuration they provide if your keyboard layout is quite different from US QWERTY). 

With a font and an input method installed, you should be able to get a semi-operational terminal with sitelen pona. The rest of the guide will concentrate on making sitelen pona recognized as [**fullwidth ideograms**](https://en.wikipedia.org/wiki/Halfwidth_and_fullwidth_forms).

### Dev tools

The following tools are **not** needed when using a sitelen pona terminal, but are required for completing the initial setup:

* a C compiler (it can be either [gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/))
* [make](https://www.gnu.org/software/make/) (a widely-used build tool)
* [git](https://git-scm.com/) (to clone the tty_pona repo) [or alternatively download it as a [zip file](https://github.com/polijan/tty_pona/archive/refs/heads/main.zip)].

(Those tools are common and may be present on your system already. Their installation depends on your actual distribution, but for example as a "sudo-trusted" user on a *Debian* or *Ubuntu*-based system, you could install everything by typing: `sudo apt install git build-essential`)

### Just one more thing...

Of course, you will also need... a  [terminal emulator](https://en.wikipedia.org/wiki/Terminal_emulator). I had success with several terminals, but not all will work. This is because, several "modern" terminals decide they know Unicode better than you(r system) and bypass some standard library calls to re-implement their own things instead. It is usually not possible to make sitelen pona work properly on such terminals (without altering their source code).

As you start your own testing, **it is recommended that you install `xterm`**, a popular and standard-compliant well-working full-featured terminal (sixels, etc). xterm is convenient as it is easy to specify a font to use just for wide characters. This is practical for using an UCSUR font specifically for sitelen pona while using your favorite usual font for ASCII and Latin.


## Installing and Using the **tty_pona** tools

### Create `tty_pona.so`

To consider sitelen pona characters as ideograms, both the terminal and the applications (which have no knowledge of fonts) need to have the same understanding of which characters consume one vs two columns on the display. To that effect, in the Unix-like world, programs should use the `wcwidth()` and `wcswidth()` functions from [POSIX](https://en.wikipedia.org/wiki/POSIX "POSIX"):

> `int wcwidth(wchar_t c);`  
>   
> The function returns the **number of columns needed to represent the character** `c`.  
> If `c` is a printable character, the value is at least 0.  
> If `c` is the null character `L'\0'`, the value is 0.  
> Otherwise, -1 is returned.  

Your system's `wcwidth` has no knowledge of sitelen pona UCSUR glyphs. Thus we must provide and expose an alternative implementation for applications to use instead of the system's C library. This is what `src/tty_pona.c` does, so **let's build the code**:

 1. Clone the repo with git: `git clone https://github.com/polijan/tty_pona.git`
 2. Build the code: `make`

This will compile:

 1.  a *shared object* `tty_pona.so` (in `lib/` folder). This is the critical component.
 2.  a test program `width` (in `bin/` folder). This program can output the length (in terminal columns) of a string passed as its argument.

Let's use...

### Using  `tty_pona.so`

In Linux, the [dynamic loader](https://en.wikipedia.org/wiki/Dynamic_loading) will first load shared libraries that are specified in the `LD_PRELOAD` environment variable before any other library. Thus by setting `LD_PRELOAD` to (the full path of) `tty_pona.so`, we can easily inject our custom `wcwidth` function in place of the standard C library to any dynamically linked executable. Let's try injecting `tty_pona.so` to the `width` program:

**TODO**

If you get the same results, you are ready to try on a terminal:

> The script **`xterm-pona`** (in the `bin/` folder) is a wrapper around xterm which "preloads" `tty_pona.so`  
> into an xterm's environment making it fully functional with sitelen pona.  
> Edit and customize the start of the script, according to what fonts you have and try it.
> Launch!
> In the shell, you can go "play" in the `ijo/` folder which contains stuff with sitelen pona.

Hopefully, you have success here!
If so, then you can play with more fonts, more terminals, etc.

## Things to consider / Troubleshooting

**TODO**


## Conclusion

I hope you enjoy using sitelen pona in terminals.

Hopefully, sitelen pona would become a part of Unicode in the future, as this would avoid having to jump through hoops for what should be a simple task. The Toki Pona language exists, has been growing, is actively used, and it is not going away any time soon. There are constantly texts being produced in it. It deserves to be encoded. Whether that will happen is not certain...

## License (MIT)

This work is **free** user the MIT License.

> Copyright (c) 2023  [jan Polijan](https://github.com/polijan)
>
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE. 
