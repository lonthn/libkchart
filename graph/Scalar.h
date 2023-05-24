//
// Created by luo-zeqi on 2013/4/19.
//

#ifndef LIBKCHART_SCALAR_H
#define LIBKCHART_SCALAR_H

namespace kchart {

// 用于衡量图形各维的参数
typedef int Scalar;

struct Size
{
    Scalar width;
    Scalar height;
};

struct Point
{
    Scalar x;
    Scalar y;
};

struct Rect
{
    Scalar left;
    Scalar top;
    Scalar right;
    Scalar bottom;

    inline Scalar Width() {
        return right - left;
    }

    inline Scalar Height() {
        return bottom - top;
    }

    inline Point Point() {
        return {left, top};
    }

    inline Size  Size() {
        return { Width(), Height() };
    }
};

}

#endif //LIBKCHART_SCALAR_H
