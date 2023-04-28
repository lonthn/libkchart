//
// Created by luo-zeqi on 2013/4/18.
//

#include "Struct.h"

namespace kchart {

DataSet::DataSet()
: rowCount(0)
{
}

ColumnKey* DataSet::AddCol(const Str& name)
{
    if (name.empty())
        return nullptr;

    auto iter = colKeys.find(name);
    if (iter != colKeys.end())
        return &iter->second;

    colKeys.emplace(name, ColumnKey{name, ColCount()});
    cols.emplace_back(rowCount);

    return &colKeys[name];
}

void DataSet::AddRow(int n)
{
    for (auto & col : cols) {
        col.resize(col.size() + n);
    }

    rowCount += n;
}

ColumnKey* DataSet::FindCol(const Str& name)
{
    auto iter = colKeys.find(name);
    if (iter != colKeys.end())
        return &iter->second;

    return nullptr;
}

DataType DataSet::Get(int col, int row)
{
    assert(col < (int) cols.size());
    assert(row < rowCount);
    return cols[col][row];
}

DataType DataSet::Get(const Index2& idx)
{
    assert(idx.col < (int) cols.size());
    assert(idx.row < rowCount);
    return cols[idx.col][idx.row];
}

void DataSet::Set(int col, int row, DataType val)
{
    cols[col][row] = val;
}

void DataSet::Set(const Index2& idx, DataType val)
{
    cols[idx.col][idx.row] = val;
}


DataRows& DataSet::operator[](int col)
{
    return cols[col];
}

}