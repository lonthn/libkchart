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

#include "StrUtils.h"

#include <cmath>
#include <cstdarg>

namespace kchart {

CStringW DataToStr(
    DataType val,
    int precision,
    int decimals
)
{
  wchar_t txt[64] = {0};
  DataType number = val / precision;
  DataType decima = val % precision;

  int len = swprintf_s(txt, 64 - 1, L"%lld", number);
  if (decimals <= 0)
    return CStringW(txt);

  txt[len++] = '.';

  int decimal_start = len;
  swprintf_s(&txt[len], 64 - len - 1, L"%lld", decima);

  for (int i = decimal_start; i < decimal_start + decimals; i++)
    if (txt[i] == '\0') txt[i] = '0';

  txt[len + decimals] = '\0';
  return CStringW(txt);
}

void StrSplit(
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