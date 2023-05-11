//
// Created by luo-zeqi on 2020/10/16.
//

#include <cassert>
#include <locale>
#include <codecvt>
#include <algorithm>

#include "WStr.h"

#ifdef WIN32
#define IsNan _isnan
#else
#define IsNan isnan
#endif

namespace kchart {

static inline bool str_start_with(const WChar* str, size_t str_len,
                                  const WChar* prefix, size_t prefix_len) {
    assert(str);
    assert(prefix);
    if (str_len < prefix_len) return false;
    return memcmp(str, prefix, prefix_len) == 0;
}

static inline bool str_end_with(const WChar* str, size_t str_len,
                                const WChar* suffix, size_t suffix_len) {
    assert(str);
    assert(suffix);
    if (str_len < suffix_len) return false;
    return memcmp(str + (str_len-suffix_len), suffix, suffix_len) == 0;
}

static inline void str_replace(std::wstring& str,
                               const WChar* src, size_t src_len,
                               const WChar* dst, size_t dst_len) {
    assert(src);
    assert(dst);
    std::wstring::size_type pos = 0;
    while ((pos = str.find(src, pos)) != std::wstring::npos) {
        str.replace(pos, src_len, dst, dst_len);
        pos += dst_len;
    }
}

static void str_split(const WChar* str, const WChar* delis,
                      bool ignore_empty_str, std::vector<WStr>& out) {
    if (ignore_empty_str) {
        str += wcsspn(str, delis);
    }
    if (!*str) {
        return;
    }

    while (true) {
        const size_t len = wcscspn(str, delis);
        if (!ignore_empty_str || len > 0) {
            out.emplace_back(str, len);
            str += len;
        }

        if (!*str) {
            return;
        }
        if (ignore_empty_str) {
            str += wcsspn(str, delis);
        } else {
            str += 1;
        }
    }
}

WStr::WStr()
    : std::wstring() {
}

WStr::WStr(const std::wstring& str)
    : std::wstring(str) {
}

bool WStr::startWith(WChar prefixChar) const {
    return str_start_with(c_str(), length(), &prefixChar, 1);
}

bool WStr::startWith(const WStr& prefixStr) const {
    return str_start_with(c_str(), length(), prefixStr.c_str(), prefixStr.length());
}

bool WStr::endWith(WChar suffixChar) const {
    return str_end_with(c_str(), length(), &suffixChar, 1);
}

bool WStr::endWith(const WStr& suffixStr) const {
    return str_end_with(c_str(), length(), suffixStr.c_str(), suffixStr.length());
}

std::vector<WStr> WStr::split(WChar deli, bool ignoreEmptyStr) const {
    std::vector<WStr> out;
    WChar delis[2] = {deli, 0};
    str_split(c_str(), delis, ignoreEmptyStr, out);
    return out;
}

std::vector<WStr> WStr::split(const WStr& delis, bool ignoreEmptyStr) const {
    std::vector<WStr> out;
    str_split(c_str(), delis.c_str(), ignoreEmptyStr, out);
    return out;
}

WStr& WStr::replace(WChar srcChar, WChar dstChar) {
    str_replace(*this, &srcChar, 1, &dstChar, 1);
    return *this;
}

WStr& WStr::replace(const WStr& srcStr, WChar dstChar) {
    str_replace(*this, srcStr.c_str(), srcStr.length(), &dstChar, 1);
    return *this;
}

WStr& WStr::replace(WChar srcChar, const WStr& dstStr) {
    str_replace(*this, &srcChar, 1, dstStr.c_str(), dstStr.length());
    return *this;
}

WStr& WStr::replace(const WStr& srcStr, const WStr& dstStr) {
    str_replace(*this, srcStr.c_str(), srcStr.length(),
                       dstStr.c_str(), dstStr.length());
    return *this;
}

WStr& WStr::toUpper() {
    std::transform(begin(), end(), begin(), toupper);
    return *this;
}

WStr& WStr::toLower() {
    std::transform(begin(), end(), begin(), tolower);
    return *this;
}

int WStr::toInt() const {
    return (int)std::wcstol(c_str(), &endPtr_, 10);
}

unsigned int WStr::toUInt() const {
    return (unsigned int)std::wcstoul(c_str(), &endPtr_, 10);
}

long WStr::toLong() const {
    return std::wcstol(c_str(), &endPtr_, 10);
}

unsigned long WStr::toULong() const {
    return std::wcstoul(c_str(), &endPtr_, 10);
}

long long WStr::toLLong() const {
    return std::wcstoll(c_str(), &endPtr_, 10);
}

unsigned long long WStr::toULLong() const {
    return std::wcstoull(c_str(), &endPtr_, 10);
}

float WStr::toFloat() const {
    return std::wcstof(c_str(), &endPtr_);
}

double WStr::toDouble() const {
    return std::wcstod(c_str(), &endPtr_);
}

long double WStr::toLDouble() const {
    return std::wcstold(c_str(), &endPtr_);
}

std::string WStr::toUTF8String() const {
//    std::wstring_convert<std::codecvt_utf8<wchar_t>> cnv;
//    return cnv.to_bytes(*this);
    return "";
}

WStr WStr::ToLower(const WStr &str) {
    WStr temp = str;
    std::transform(temp.begin(), temp.end(), temp.begin(), tolower);
    return temp;
}

WStr WStr::FromUTF8(const char* utf8) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cnv;
    return cnv.from_bytes(utf8);
    //return L"";
}

WStr WStr::FromUTF8(const std::string& utf8) {
    return FromUTF8(utf8.c_str());
}

WStr WStr::ToUpper(const WStr &str) {
    WStr temp = str;
    std::transform(temp.begin(), temp.end(), temp.begin(), toupper);
    return temp;
}

WStr WStr::ToString(double val, int digit) {
    if (!IsNan(val)) {
        if (digit == 0) {
            double newValue = round(val);
            if (abs(val - newValue) < 1) val = newValue;
        }
        wchar_t str[64];
        switch (digit) {
            case 0: swprintf(str, 63, L"%d", (int)val); break;
            case 1: swprintf(str, 63, L"%.1f", val); break;
            case 2: swprintf(str, 63, L"%.2f", val); break;
            case 3: swprintf(str, 63, L"%.3f", val); break;
            case 4: swprintf(str, 63, L"%.4f", val); break;
            case 5: swprintf(str, 63, L"%.5f", val); break;
            case 6: swprintf(str, 63, L"%.6f", val); break;
            case 7: swprintf(str, 63, L"%.7f", val); break;
            case 8: swprintf(str, 63, L"%.8f", val); break;
            case 9: swprintf(str, 63, L"%.9f", val); break;
            default: swprintf(str, 63, L"%f", val); break;
        }

        return str;
    }

    return L"";
}

WStr WStr::ToString(int32_t val) {
    return WStr(std::to_wstring(val));
}

WStr WStr::ToString(uint32_t val) {
    return WStr(std::to_wstring(val));
}

WStr WStr::ToString(int64_t val) {
    return WStr(std::to_wstring(val));
}

WStr WStr::ToString(uint64_t val) {
    return WStr(std::to_wstring(val));
}

WStr WStr::ToString(float val) {
    return WStr(std::to_wstring(val));
}

WStr WStr::ToString(double val) {
    return WStr(std::to_wstring(val));
}

WStr WStr::ToString(long double val) {
    return WStr(std::to_wstring(val));
}

}