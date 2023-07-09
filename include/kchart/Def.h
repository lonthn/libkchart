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

#ifndef LIBKCHART_DEF_H
#define LIBKCHART_DEF_H

#include <cstdint>
#include <cassert>

#include <vector>

// 通常来讲，用于展示的数据不可能达到 int64 的最大值
// 所以我们用它来表示无意义的数据.
#define KC_INVALID_DATA INT64_MAX //0x7FFFFFFFFFFFFFFF

namespace kchart {

typedef int  GraphicsId;

typedef int64_t  DataType;
typedef uint32_t Color;

typedef std::vector<DataType>  DataRows;
typedef std::vector<DataRows>  DataCols;

}

#endif //LIBKCHART_DEF_H