//
// Created by luo-zeqi on 2013/4/19.
//

#ifndef LIBKCHART_SCALAR_H
#define LIBKCHART_SCALAR_H

namespace kchart {

// ���ں���ͼ�θ�ά�Ĳ���
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
