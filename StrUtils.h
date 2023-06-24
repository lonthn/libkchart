//
// Created by luo-zeqi on 2020/10/16.
//

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
