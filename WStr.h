//
// Created by luo-zeqi on 2020/10/16.
//

#ifndef LIBKCHART_WSTR_H
#define LIBKCHART_WSTR_H

#include <string>
#include <vector>
#include <cmath>
#include <cstdarg>

namespace kchart {

typedef wchar_t WChar;

class WStr : public std::wstring {
public:
    using std::wstring::wstring;
    using std::wstring::operator=;
    using std::wstring::substr;
    using std::wstring::replace;

    WStr();
    WStr(const std::wstring& str);

public:
    bool startWith(WChar prefixChar) const;
    bool startWith(const WStr& prefixStr) const;
    bool endWith(WChar suffixChar) const;
    bool endWith(const WStr& suffixStr) const;

    std::vector<WStr> split(WChar delis, bool ignoreEmptyStr = true) const;
    std::vector<WStr> split(const WStr& delis, bool ignoreEmptyStr = true) const;

    WStr& replace(WChar srcChar, WChar dstChar);
    WStr& replace(const WStr& srcStr, WChar dstChar);
    WStr& replace(WChar srcChar, const WStr& dstStr);
    WStr& replace(const WStr& srcStr, const WStr& dstStr);

    WStr& toUpper();
    WStr& toLower();

    int                 toInt() const;
    unsigned int        toUInt() const;
    long                toLong() const;
    unsigned long       toULong() const;
    long long           toLLong() const;
    unsigned long long  toULLong() const;
    float               toFloat() const;
    double              toDouble() const;
    long double         toLDouble() const;
    wchar_t* endPtr() const {
        return endPtr_;
    }

    std::string toUTF8String() const;

    static WStr FromUTF8(const char* utf8);
    static WStr FromUTF8(const std::string& utf8);
    static WStr ToUpper(const WStr& str);
    static WStr ToLower(const WStr& str);
    static WStr ToString(double val, int digit);
    static WStr ToString(int32_t val);
    static WStr ToString(uint32_t val);
    static WStr ToString(int64_t val);
    static WStr ToString(uint64_t val);
    static WStr ToString(float val);
    static WStr ToString(double val);
    static WStr ToString(long double val);

private:
    mutable wchar_t* endPtr_ = nullptr;
};

static WStr StringFormat(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);

    wchar_t out[256];
    int len = vswprintf(out, 256, format, args);

    va_end(args);
    return WStr(out, len);
}

}

#endif //LIBKCHART_WSTR_H
