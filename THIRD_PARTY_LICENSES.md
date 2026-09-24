# Third-party licenses and credits

This project uses the third-party materials listed below. Each retains its own
license or public-domain notice; any license for this project's original work
does not replace these terms. Notices in the source files are retained.

## Open Color

The palette values in `src/render/palette.h` come from
[Open Color](https://github.com/yeun/open-color).
[Upstream license](https://github.com/yeun/open-color/blob/master/LICENSE):

```text
MIT License

Copyright (c) 2016 heeyeun

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## DINOBYTE bitmap font

This game uses the [DINOBYTE bitmap font](https://mby.itch.io/dinobyte) by
[mby](https://mby.itch.io/), licensed under
[Creative Commons Attribution 4.0 International (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/).
The [full license terms](https://creativecommons.org/licenses/by/4.0/legalcode)
apply to the font.

The font file, `dbyte_1x.png`, is downloaded separately into `assets/` and is
included in game builds as `res/dbyte_1x.png`. It is not bundled in this source
repository.

The downloaded PNG file is used unchanged. At runtime, the game converts the
font's RGB pixel values to white, preserving transparency, so that glyphs can
be tinted during rendering.

## Random-number generation

`libs/core/rand.c` adapts
[xoshiro256**](https://prng.di.unimi.it/xoshiro256starstar.c), written in 2018
by David Blackman and Sebastiano Vigna, and uses
[SplitMix64](https://prng.di.unimi.it/splitmix64.c), written in 2015 by
Sebastiano Vigna, to initialize the generator state.

Both upstream implementations carry the following public-domain dedication,
permission, and disclaimer. The original xoshiro256** notice is also retained
in `libs/core/rand.c`.

```text
Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)
```

```text
Written in 2015 by Sebastiano Vigna (vigna@acm.org)
```

```text
To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
```

## SDL3

This game uses [Simple DirectMedia Layer (SDL3)](https://www.libsdl.org/),
built from separately obtained source. SDL is licensed under the zlib license.
The following notice comes from the local SDL dependency's `LICENSE.txt`;
the [upstream license](https://github.com/libsdl-org/SDL/blob/main/LICENSE.txt)
is also available online.

```text
Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
```
