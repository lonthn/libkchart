//
// Created by luo-zeqi on 2020/10/16.
//

#ifndef LIBKCHART_STR_H
#define LIBKCHART_STR_H

#include <string>
#include <vector>
#include <cstdarg>

namespace kchart {

typedef char CharT;

class Str : public std::string {
public:
    using std::string::string;
    using std::string::operator=;
    using std::string::substr;
    using std::string::replace;

    Str();

    Str(const std::string &str);

public:
    bool startWith(CharT prefixChar) const;

    bool startWith(const Str &prefixStr) const;

    bool endWith(CharT suffixChar) const;

    bool endWith(const Str &suffixStr) const;

    std::vector<Str> split(CharT delis, bool ignoreEmptyStr = true) const;

    std::vector<Str> split(const Str &delis, bool ignoreEmptyStr = true) const;

    Str &replace(CharT srcChar, CharT dstChar);

    Str &replace(const Str &srcStr, CharT dstChar);

    Str &replace(CharT srcChar, const Str &dstStr);

    Str &replace(const Str &srcStr, const Str &dstStr);

    Str &toUpper();

    Str &toLower();

    int toInt() const;

    unsigned int toUInt() const;

    long toLong() const;

    unsigned long toULong() const;

    long long toLLong() const;

    unsigned long long toULLong() const;

    float toFloat() const;

    double toDouble() const;

    long double toLDouble() const;

    char *endPtr() const {
        return endPtr_;
    }

    std::string toUTF8String() const;

    static Str FromUTF8(const char *utf8);

    static Str FromUTF8(const std::string &utf8);

    static Str ToUpper(const Str &str);

    static Str ToLower(const Str &str);

    static Str ToString(double val, int digit);

    static Str ToString(int32_t val);

    static Str ToString(uint32_t val);

    static Str ToString(int64_t val);

    static Str ToString(uint64_t val);

    static Str ToString(float val);

    static Str ToString(double val);

    static Str ToString(long double val);

private:
    mutable CharT *endPtr_ = nullptr;
};

static Str StringFormat(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char out[256];
    vsprintf_s(out, 255, format, args);

    va_end(args);
    return Str(out);
}

}

#endif //LIBKCHART_STR_H
