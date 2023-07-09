// MIT License
//
// Copyright (c) 2023 luo-zeqi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef LIBKCHART_COLOR_H
#define LIBKCHART_COLOR_H

#include <cstdint>

typedef uint32_t Color;

#define ColorGetA(color)    (((color) >> 24u) & 0xffu)
#define ColorGetR(color)    (((color) >> 16u) & 0xffu)
#define ColorGetG(color)    (((color) >>  8u) & 0xffu)
#define ColorGetB(color)    ((color) & 0xffu)

inline Color MakeColor(uint8_t r,
                       uint8_t g,
                       uint8_t b,
                       uint8_t a = 255) {
  return ((Color) (
      (((uint32_t) a) << 24u) |
      (((uint32_t) r) << 16u) |
      (((uint32_t) g) << 8u) |
      b)
  );
}

#endif // LIBKCHART_COLOR_H
