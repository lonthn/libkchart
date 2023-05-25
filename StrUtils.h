//
// Created by luo-zeqi on 2020/10/16.
//

#ifndef LIBKCHART_STRUTILS_H
#define LIBKCHART_STRUTILS_H

#include <vector>
#include <cmath>
#include <cstdarg>
#include <atlstr.h>

namespace kchart {

static CStringW DoubleToStr(double val, int digit)
{
  if (_isnan(val))
    return {};

  if (digit == 0) {
    double newValue = round(val);
    if (abs(val - newValue) < 1) val = newValue;
  }

  CStringW str;
  switch (digit) {
    case 0:  str.Format(L"%d", (int)val); break;
    case 1:  str.Format(L"%.1f", val); break;
    case 2:  str.Format(L"%.2f", val); break;
    case 3:  str.Format(L"%.3f", val); break;
    case 4:  str.Format(L"%.4f", val); break;
    case 5:  str.Format(L"%.5f", val); break;
    case 6:  str.Format(L"%.6f", val); break;
    case 7:  str.Format(L"%.7f", val); break;
    case 8:  str.Format(L"%.8f", val); break;
    case 9:  str.Format(L"%.9f", val); break;
    default: str.Format(L"%f", val);   break;
  }

  return str;
}

static void StrSplit(
    const char *str,
    const char *delis,
    bool ignore_space,
    std::vector<CStringA>& out
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
