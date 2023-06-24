//
// Created by luo-zeqi on 2013/4/17.
//

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