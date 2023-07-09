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

#ifndef LIBKCHART_SCALAR_H
#define LIBKCHART_SCALAR_H

namespace kchart {

// 用于衡量图形各维的参数
typedef int Scalar;

struct Size {
  Scalar width;
  Scalar height;
};

struct Point {
  Scalar x;
  Scalar y;
};

struct Rect {
  Scalar left;
  Scalar top;
  Scalar right;
  Scalar bottom;

  inline Scalar Width() const {
    return right - left;
  }

  inline Scalar Height() const {
    return bottom - top;
  }

  inline Point Point() const {
    return {left, top};
  }

  inline Size Size() const {
    return {Width(), Height()};
  }
};

}

#endif //LIBKCHART_SCALAR_H
