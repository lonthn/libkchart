//
// Created by luo-zeqi on 2020/10/16.
//

#include "Str.h"

#include <cassert>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <cstring>
#include <cmath>

#ifdef WIN32
#define IsNan _isnan
#else
#define IsNan isnan
#endif

namespace kchart {

static inline bool str_start_with(const CharT *str, size_t str_len,
                                  const CharT *prefix, size_t prefix_len) {
    assert(str);
    assert(prefix);
    if (str_len < prefix_len) return false;
    return memcmp(str, prefix, prefix_len) == 0;
}

static inline bool str_end_with(const CharT *str, size_t str_len,
                                const CharT *suffix, size_t suffix_len) {
    assert(str);
    assert(suffix);
    if (str_len < suffix_len) return false;
    return memcmp(str + (str_len - suffix_len), suffix, suffix_len) == 0;
}

static inline void str_replace(std::string &str,
                               const CharT *src, size_t src_len,
                               const CharT *dst, size_t dst_len) {
    assert(src);
    assert(dst);
    std::string::size_type pos = 0;
    while ((pos = str.find(src, pos)) != std::string::npos) {
        str.replace(pos, src_len, dst, dst_len);
        pos += dst_len;
    }
}

static void str_split(const CharT *str, const CharT *delis,
                      bool ignore_space, std::vector<Str> &out) {
//    if (ignore_space) {
//        str += strspn(str, " ");
//    }
    if (!*str) {
        return;
    }

    int delislen = strlen(delis);

    while (true) {
        const char *s = strstr(str, delis);
        if (!s) {
            out.emplace_back(str);
            return;
        }

        if (s == str && !ignore_space) {
            s += delislen;
            continue;
        }

        out.emplace_back(str, s - str);
        str = s + delislen;

        if (!*str) {
            return;
        }
    }
}

Str::Str()
: std::string() {
}

Str::Str(const std::string &str)
: std::string(str) {
}

bool Str::startWith(CharT prefixChar) const {
    return str_start_with(c_str(), length(), &prefixChar, 1);
}

bool Str::startWith(const Str &prefixStr) const {
    return str_start_with(c_str(), length(), prefixStr.c_str(), prefixStr.length());
}

bool Str::endWith(CharT suffixChar) const {
    return str_end_with(c_str(), length(), &suffixChar, 1);
}

bool Str::endWith(const Str &suffixStr) const {
    return str_end_with(c_str(), length(), suffixStr.c_str(), suffixStr.length());
}

std::vector<Str> Str::split(CharT deli, bool ignoreEmptyStr) const {
    std::vector<Str> out;
    CharT delis[2] = {deli, 0};
    str_split(c_str(), delis, ignoreEmptyStr, out);
    return out;
}

std::vector<Str> Str::split(const Str &delis, bool ignoreEmptyStr) const {
    std::vector<Str> out;
    str_split(c_str(), delis.c_str(), ignoreEmptyStr, out);
    return out;
}

Str &Str::replace(CharT srcChar, CharT dstChar) {
    str_replace(*this, &srcChar, 1, &dstChar, 1);
    return *this;
}

Str &Str::replace(const Str &srcStr, CharT dstChar) {
    str_replace(*this, srcStr.c_str(), srcStr.length(), &dstChar, 1);
    return *this;
}

Str &Str::replace(CharT srcChar, const Str &dstStr) {
    str_replace(*this, &srcChar, 1, dstStr.c_str(), dstStr.length());
    return *this;
}

Str &Str::replace(const Str &srcStr, const Str &dstStr) {
    str_replace(*this, srcStr.c_str(), srcStr.length(),
                dstStr.c_str(), dstStr.length());
    return *this;
}

Str &Str::toUpper() {
    std::transform(begin(), end(), begin(), toupper);
    return *this;
}

Str &Str::toLower() {
    std::transform(begin(), end(), begin(), tolower);
    return *this;
}

int Str::toInt() const {
    return (int) std::strtol(c_str(), &endPtr_, 10);
}

unsigned int Str::toUInt() const {
    return (unsigned int) std::strtoul(c_str(), &endPtr_, 10);
}

long Str::toLong() const {
    return std::strtol(c_str(), &endPtr_, 10);
}

unsigned long Str::toULong() const {
    return std::strtoul(c_str(), &endPtr_, 10);
}

long long Str::toLLong() const {
    return std::strtoll(c_str(), &endPtr_, 10);
}

unsigned long long Str::toULLong() const {
    return std::strtoull(c_str(), &endPtr_, 10);
}

float Str::toFloat() const {
    return std::strtof(c_str(), &endPtr_);
}

double Str::toDouble() const {
    return std::strtod(c_str(), &endPtr_);
}

long double Str::toLDouble() const {
    return std::strtold(c_str(), &endPtr_);
}

std::string Str::toUTF8String() const {
    // std::wstring_convert<std::codecvt_utf8<wchar_t>> cnv;
    // return cnv.to_bytes(*this);
    return "";
}

Str Str::ToLower(const Str &str) {
    Str temp = str;
    std::transform(temp.begin(), temp.end(), temp.begin(), tolower);
    return temp;
}

Str Str::FromUTF8(const char *utf8) {
//    std::wstring_convert<std::codecvt_utf8<wchar_t>> cnv;
//    return cnv.from_bytes(utf8);
    return "";
}

Str Str::FromUTF8(const std::string &utf8) {
    return FromUTF8(utf8.c_str());
}

Str Str::ToUpper(const Str &str) {
    Str temp = str;
    std::transform(temp.begin(), temp.end(), temp.begin(), toupper);
    return temp;
}

Str Str::ToString(double val, int digit) {
    if (IsNan(val))
        return "nan";

    if (digit == 0) {
        double newValue = round(val);
        if (abs(val - newValue) < 1) val = newValue;
    }

    CharT str[64];
    switch (digit) {
        case 0:
            sprintf_s(str, 63, "%d", (int) val);
            break;
        case 1:
            sprintf_s(str, 63, "%.1f", val);
            break;
        case 2:
            sprintf_s(str, 63, "%.2f", val);
            break;
        case 3:
            sprintf_s(str, 63, "%.3f", val);
            break;
        case 4:
            sprintf_s(str, 63, "%.4f", val);
            break;
        case 5:
            sprintf_s(str, 63, "%.5f", val);
            break;
        case 6:
            sprintf_s(str, 63, "%.6f", val);
            break;
        case 7:
            sprintf_s(str, 63, "%.7f", val);
            break;
        case 8:
            sprintf_s(str, 63, "%.8f", val);
            break;
        case 9:
            sprintf_s(str, 63, "%.9f", val);
            break;
        default:
            sprintf_s(str, 63, "%f", val);
            break;
    }

    return str;
}

Str Str::ToString(int32_t val) {
    return Str(std::to_string(val));
}

Str Str::ToString(uint32_t val) {
    return Str(std::to_string(val));
}

Str Str::ToString(int64_t val) {
    return Str(std::to_string(val));
}

Str Str::ToString(uint64_t val) {
    return Str(std::to_string(val));
}

Str Str::ToString(float val) {
    return Str(std::to_string(val));
}

Str Str::ToString(double val) {
    return Str(std::to_string(val));
}

Str Str::ToString(long double val) {
    return Str(std::to_string(val));
}

}