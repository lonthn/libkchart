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

#ifndef LIBKCHART_STRUTILS_H
#define LIBKCHART_STRUTILS_H

#include <vector>
#include <cmath>
#include <cstdarg>

namespace kchart {

static CStringW DataToStr(
    DataType val,
    int precision,
    int decimals
)
{
#define FORMAT_OUT(format_str) \
  str.Format(L##format_str, double(val)/double(precision))

  CStringW str;
  switch (decimals) {
    case 0: str.Format(L"%lld", val/precision); break;
    case 1: FORMAT_OUT("%.01f"); break;
    case 2: FORMAT_OUT("%.02f"); break;
    case 3: FORMAT_OUT("%.03f"); break;
    case 4: FORMAT_OUT("%.04f"); break;
    case 5: FORMAT_OUT("%.05f"); break;
    case 6: FORMAT_OUT("%.06f"); break;
    case 7: FORMAT_OUT("%.07f"); break;
    case 8: FORMAT_OUT("%.08f"); break;
    case 9: FORMAT_OUT("%.09f"); break;
    default: str.Format(L"%lld", val/precision);
  }
  return str;
}

static void StrSplit(
    const char *str,
    const char *delis,
    bool ignore_space,
    std::vector<std::string>& out
)
{
  if (!*str)
      return;

  int delislen = (int) strlen(delis);

  while (true) {
    if (ignore_space && *str == ' ') {
      str++;
      continue;
    }
    const char *s = strstr(str, delis);
    if (!s) {
      out.emplace_back(str);
      return;
    }

    if (s == str && ignore_space) {
      str += delislen;
      continue;
    }

    int len = static_cast<int>(s - str);
    out.emplace_back(str, len);
    str = s + delislen;

    if (!*str)
      return;
  }
}

}

#endif //LIBKCHART_STRUTILS_H
