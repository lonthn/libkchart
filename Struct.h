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

struct ColumnKey
{
    Str name;
    int index;
};

struct Index2
{
    int row;
    int col;
};

struct DataSet
{
public:
    DataSet();

    ColumnKey* AddCol(const Str& name);
    void AddRow(int n);

    ColumnKey* FindCol(const Str& name);

    inline int RowCount() const { return rowCount; }
    inline int ColCount() const { return (int) cols.size(); }

    DataType Get(int col, int row);
    DataType Get(const Index2& idx);
    void     Set(int col, int row, DataType val);
    void     Set(const Index2& idx, DataType val);

    DataRows& operator[](int col);

private:
    DataCols cols;
    int rowCount;
    std::map<Str, ColumnKey> colKeys;
};

class DrawData
{
public:
    DrawData(
        DataSet& data, int off, int count,
        const Size& size, float wr, float hr,
        DataType bias
    )
    : data_(data)
    , off_(off)
    , count_(count)
    , size_(size)
    , wratio_(wr)
    , hratio_(hr)
    , bias_(bias)
    { }

    int Count() const {
        return count_;
    }

    inline DataType& Get(int row, int col) {
        return data_[col][off_ + row];
    }

    inline DataType& Get(const Index2& idx) {
        return data_[idx.col][off_ + idx.row];
    }

    inline Scalar ToPY(DataType val) const {
        return size_.height - Scalar(hratio_ * float(val - bias_));
    }
    inline float GetWRatio() const {
        return wratio_;
    }
    inline Size GetSize() const {
        return size_;
    }

private:
    DataSet& data_;
    int off_;
    int count_;

    Size size_;
    float wratio_;
    float hratio_;
    DataType bias_;
};

}

#endif //LIBKCHART_STRUCT_H
