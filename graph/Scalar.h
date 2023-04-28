//
// Created by luo-zeqi on 2013/4/19.
//

#ifndef LIBKCHART_SCALAR_H
#define LIBKCHART_SCALAR_H

namespace kchart {

// ���ں���ͼ�θ�ά�Ĳ���
typedef int Scalar;

struct Rect
{
    Scalar left;
    Scalar top;
    Scalar right;
    Scalar bottom;
};

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

}

#endif //LIBKCHART_SCALAR_H
