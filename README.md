# tty_pona

> pali ni li pana e nasin e ken ni:  
> ilo nanpa la, sina ken kepeken sitelen pona lon poki pimeja ilo pi pana nimi.

## TL;DR

This project provides some guidance and necessary tools to setup and use ***sitelen pona*** (script for the constructed language [**toki pona**](https://en.wikipedia.org/wiki/Toki_Pona) [[tok](https://iso639-3.sil.org/code/tok)]) as proper **fullwidth** ideograms **in terminal emulators** under Unix-like systems (mostly *Linux*).

![a text editor running in a terminal emulator with sitelen pona properly setup as fullwidth ideograms](https://cdn.discordapp.com/attachments/994960433951801374/1162480696443535505/image.png)

## Prerequisites

### UCSUR-compatible font(s)

As of today, the **sitelen pona** script is _not_ present in [Unicode](https://en.wikipedia.org/wiki/Unicode). However, since 2021, the [Under-ConScript Unicode Registry](https://kreativekorp.com/ucsur/) (**UCSUR**) provides a de-facto standard encoding for it in the block `U+F1900` - `U+F19FF` of the [Private Use Areas](https://en.wikipedia.org/wiki/Private_Use_Areas) (PUA) of Unicode. We're going to use this encoding, so you may want to browse through [**the specification**](https://www.kreativekorp.com/ucsur/charts/sitelen.html). 

Several [sitelen pona fonts](https://docs.google.com/spreadsheets/d/1xwgTAxwgn4ZAc4DBnHte0cqta1aaxe112Wh1rv9w5Yk/edit#gid=1195574771)  (Truetype or Opentype) are now using this UCSUR encoding, but old fonts aren't (so beware). Eventually, you'll want to download and try a few different fonts (perhaps from [this "Linku" repo](https://github.com/lipu-linku/nasin-sitelen)), but to quickly begin testing I recommend you start with the [Fairfax HD](https://www.kreativekorp.com/software/fonts/fairfaxhd.shtml) font. Install the font(s) on your system (perhaps locally in `~/.local/share/fonts`).

> Assuming your system uses fonconfig, to verify your font installation and list which sitelen pona UCSUR fonts are recognized by your system, you can use the `bin/fc-list-pona` script.

With font(s) properly installed, you can use sitelen pona in your desktop environment, mobile device, *Discord* messaging platform application, etc. This [guide](https://github.com/Id405/sitelen-pona-ucsur-guide/) could help. Good; but let's continue the setup required for pona-ifying the terminal ...

### An input method

You might postpone this step to the end, but to work productively at the terminal, you will require a way to easily input the sitelen pona UCSUR glyphs. Several fonts let you input latin characters and show/output sitelen pona, but it's ligatures which often won't work (quite the contrary) with terminals and not what we want. We need a "true" input method. For Unix-like systems, there are currently three main options to choose from:

 - a full [sitelen pona keyboard layout](https://www.kreativekorp.com/software/keyboards/sitelenpona/) which can directly enter all the sitelen pona UCSUR glyphs.
 - an `ibus` and `ibus-table` based method where you enter the word in Latin and it completes into the sitelen pona glyph. This is all described [here](https://github.com/Id405/sitelen-pona-ucsur-guide/#linux) and it works!
 - a method called "*Wakalito*" ([instruction and definitions direct download](https://cdn.discordapp.com/attachments/316066233755631616/1061846136907890778/nasin_sitelen_Wakalito_7.3.2.zip)), based on the free software `espanso`, where the left half of the keyboard represents shapes which you input to create sitelen pona glyphs.

(Some of those methods might require a few adjustement in their configs if your keyboard layout is quite different from US QWERTY). Anyway, with font and input method installed, you should be able to have a semi-operational terminal with sitelen pona. The rest of the guide will concentrate on making sitelen pona recognized as proper [**fullwidth ideograms**](https://en.wikipedia.org/wiki/Halfwidth_and_fullwidth_forms).

### Dev tools

The following tools are **not** needed when using a sitelen pona terminal, but are required for completing the initial setup:

* a C compiler, which can be either [gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/).
* [make](https://www.gnu.org/software/make/) (a widely-used build tool)
* [git](https://git-scm.com/) (to clone the tty_pona repo) [or alternatively download it as a [zip file](https://github.com/polijan/tty_pona/archive/refs/heads/main.zip)].

(Those tools are common and you might have them on your system already. Their installation depends on your actual system, but for example as a "sudo-trusted" user on a Debian or Ubuntu-based system, you could install everything by typing: `sudo apt install git build-essential`)

### Just one more thing...

Of course, you will also need... a  [terminal emulator](https://en.wikipedia.org/wiki/Terminal_emulator). I had success with several terminals, but not all will work. This is because, several "modern" terminals decide they know Unicode better than you(r system) and bypass some UNIX standard calls to re-implement their own things instead. (This is not a particularly clever thing to do of course as that means text applications using system libraries now have slightly different knowledge of than the terminal which may lead to glitches). It is usually not possible to make sitelen pona work properly on such terminals (without altering their source code).

As you start your own testing, **it is recommended that you install `xterm`**, a popular and standard-compliant well-working full-featured terminal (sixels, etc). xterm is convenient as it is easy to specify a font to use just for wide characters. This is practical for using an UCSUR font specifically for sitelen pona while you keep using your favorite font for ASCII and Latin.


## Let's hack !!!

### Create `tty_pona.so`

Unfortunately, simply instructing your terminal to use a sitelen pona UCSUR font is not enough. To consider sitelen pona characters as ideograms which should be displayed wider, both the terminal and the applications (which have no knowledge of fonts) need to have the same understanding of which characters consume one or two "columns". To that effect, in the Unix-like world, programs should use the `wcwidth()` and `wcswidth()` functions from [POSIX](https://en.wikipedia.org/wiki/POSIX "POSIX"):

> `int wcwidth(wchar_t c);`  
>   
> The function returns the **number of columns needed to represent the character** `c`.  
> If `c` is a printable character, the value is at least 0.  
> If `c` is the null character `L'\0'`, the value is 0.  
> Otherwise, -1 is returned.  

And here lies the problem! As UCSUR is not part of the Unicode standard, your system's `wcwidth` has no knowledge that sitelen pona glyphs should be ideographic fullwidth characters. That means we must provide and expose our own alternative implementation for applications to use instead of the system's C library. Overall, the logic of the **hijacked** `wcwdith` is really quite simple:

```C
int wcwidth(wchar_t c)
{
   /* sitelen pona UCSUR character: we say it's a full-width character... except
    * for unprintable controls (combiner and joiner controls, and "long" glyphs
    * introducers [note: we still can print `START OF LONG PI`])
    */
   if ((c >= 0xF1900ul) && (c <= 0xF19FFul)) {
      if ((c >= 0xF1992ul) && (c <= 0xF199Bul) && (c != 0xF1993ul)) {
         return -1;
      }
      return 2;
   } else {
      /* non sitelen pona characters */
      ... // <- here we should "somehow" defer back to the `wcwidth` of the C library.
   }
}
````

The full implementation is provided in the file `src/tty_pona.c` (you can have a look if you're interested about how the *"somehow"* part work). In any case, at this point, **you should**:

 1. Clone the repo with git: `git clone https://github.com/polijan/tty_pona.git`
 2. Build the code: `make`

This will compile:

 1.  the code into a *shared object* `lib/tty_pona.so`. This is the important part that makes the magic happen!
 2.  a test program called `bin/width` which output the length (in terminal columns) of a string passed as its argument.

Let's use...

### How to start using  `tty_pona.so`

In Linux, the [dynamic loader](https://en.wikipedia.org/wiki/Dynamic_loading) will first load shared libraries that are specified in the `LD_PRELOAD` environment variable before any other library. Thus by setting `LD_PRELOAD` to (the full path of) `tty_pona.so`, we can easily inject our customized `wcwidth` function in place of the standard C library to any dynamically linked executable. Let's try it.

Test injecting `tty_pona.so` using the `wdith` program:

**TODO**

Finally, let's try a terminal:

> The script **`bin/xterm-pona`** is a wrapper around xterm which "preloads" `tty_pona.so`  
> into a xterm's environment making it fully functional with sitelen pona.  
> Edit, customize it according to what fonts your have and try it (in the shell, go play with file in the `lipu` folder)  

Hopefully, you have success here!
After that, you can play with fonts, try other terminals, etc.


### Things to consider:

**TODO**


### The Future

Hopefully, sitelen pona would become part of Unicode in the future, this would avoid having to jump through hoops for what should be a simple task. The Toki Pona language exists, has been growing, is actively used, and it is not going away any time soon. There are constantly texts being produced in it. It deserves to be encoded (it doesn't even require many codepoints). Whether that will happen is not certain...

## License (MIT)

This work is **free** user the MIT License

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
