//
// Created by luo-zeqi on 2020/10/16.
//

#ifndef LIBKCHART_WSTR_H
#define LIBKCHART_WSTR_H

#include <vector>
#include <cmath>
#include <cstdarg>
#include <atlstr.h>

namespace kchart {

static CStringW DoubleToStr(double val, int digit);

static void WStrSplit(
    const wchar_t *str,
    const wchar_t *delis,
    bool ignore_space,
    std::vector<CStringW>& out
) ;

static void StrSplit(
    const char *str,
    const char *delis,
    bool ignore_space,
    std::vector<CStringA>& out
);

}

#endif //LIBKCHART_WSTR_H
