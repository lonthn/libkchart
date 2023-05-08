//
// Created by luo-zeqi on 2013/4/18.
//

#ifndef LIBKCHART_STRUCT_H
#define LIBKCHART_STRUCT_H

#include "Def.h"
#include "Str.h"
#include "graph/Scalar.h"

#include <vector>
#include <map>

namespace kchart {

typedef struct ColumnInfo
{
    Str name;
    int index;
} * ColumnKey;

struct Index2
{
    int row;
    int col;
};

struct DataSet
{
public:
    DataSet();

    ColumnKey AddCol(const Str& name);
    void      AddRow(int n);

    ColumnKey FindCol(const Str& name);

    inline int RowCount() const { return rowCount; }
    inline int ColCount() const { return (int) cols.size(); }

    DataType Get(int col, int row);
    DataType Get(const Index2& idx);
    DataType Get(ColumnKey col, int row);
    void     Set(int col, int row, DataType val);
    void     Set(const Index2& idx, DataType val);

    DataRows& operator[](int col);
    DataRows& operator[](ColumnKey col);

private:
    DataCols cols;
    int rowCount;
    std::map<Str, ColumnInfo> colKeys;
};

struct DrawData
{
public:
    Size   size;
    Scalar sWidth;
    Scalar sMargin;
    float wRatio;
    float hRatio;
    DataType bias;

    DrawData(DataSet& data, int off, int count)
    : data(data)
    , off(off)
    , count(count)
    {
        size = {};
        wRatio = 0.0;
        hRatio = 0.0;
        bias = 0;
    }

    inline int Count() const { return count; }

    inline DataType Get(int col, int row) const {
        return data[col][off + row];
    }

    inline DataType Get(ColumnKey col, int row) const {
        return data[col][off + row];
    }

    inline DataType Get(const Index2& idx) const {
        return data[idx.col][off + idx.row];
    }

    inline Scalar ToPX(int idx) const {
        return size.width - (count - idx) * sWidth + (sWidth/2+1);
    }

    inline Scalar ToPY(DataType val) const {
        return size.height - Scalar(hRatio * float(val - bias));
    }

private:
    DataSet& data;
    int off;
    int count;
};

}

#endif //LIBKCHART_STRUCT_H
