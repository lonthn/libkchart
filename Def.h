//
// Created by luo-zeqi on 2013/4/17.
//

#ifndef LIBKCHART_DEF_H
#define LIBKCHART_DEF_H

#include <cstdint>
#include <cassert>

#include <vector>

namespace kchart {

typedef int  GraphicsId;

typedef float    DataType;
typedef uint32_t Color;

typedef std::vector<DataType>  DataRows;
typedef std::vector<DataRows>  DataCols;

}

#endif //LIBKCHART_DEF_H