//
// Created by luo-zeqi on 2013/4/18.
//

#include "Struct.h"

#include <atlconv.h>

namespace kchart {

DataSet::DataSet()
    : rowCount(0) {
}

ColumnKey DataSet::AddCol(const std::string &name) {
  if (name.empty())
    return nullptr;

  auto iter = colKeys.find(name);
  if (iter != colKeys.end())
    return &iter->second;

  USES_CONVERSION;
  CStringW wname = A2W(name.c_str());
  colKeys.emplace(name, ColumnInfo{wname, ColCount()});
  cols.emplace_back(rowCount);

  return &colKeys[name];
}

int DataSet::AddRow() {
  for (auto &col: cols) {
    col.resize(col.size() + 1);
  }

  return rowCount++;
}

void DataSet::AddRow(int n) {
  for (auto &col: cols) {
    col.resize(col.size() + n);
  }

  rowCount += n;
}

ColumnKey DataSet::FindCol(const std::string &name) {
  auto iter = colKeys.find(name);
  if (iter != colKeys.end())
    return &iter->second;

  return nullptr;
}

DataType DataSet::Get(int col, int row) {
  assert(col < (int) cols.size());
  assert(row < rowCount);
  return cols[col][row];
}

DataType DataSet::Get(const Index2 &idx) {
  assert(idx.col < (int) cols.size());
  assert(idx.row < rowCount);
  return cols[idx.col][idx.row];
}

DataType DataSet::Get(ColumnKey col, int row) {
  assert(col->index < (int) cols.size());
  assert(row < rowCount);
  return cols[col->index][row];
}

void DataSet::Set(int col, int row, DataType val) {
  assert(col < (int) cols.size());
  assert(row < rowCount);
  cols[col][row] = val;

  for (auto &fn: observers) {
    fn({col, row});
  }
}

void DataSet::Set(const Index2 &idx, DataType val) {
  assert(idx.col < (int) cols.size());
  assert(idx.row < rowCount);
  cols[idx.col][idx.row] = val;

  for (auto &fn: observers) {
    fn(idx);
  }
}

void DataSet::Set(ColumnKey col, int row, DataType val) {
  assert(col->index < (int) cols.size());
  assert(row < rowCount);
  cols[col->index][row] = val;

  for (auto &fn: observers) {
    fn({col->index, row});
  }
}

DataRows &DataSet::Get(ColumnKey col) {
  assert(col->index < (int) cols.size());
  return cols[col->index];
}

DataRows &DataSet::operator[](int col) {
  assert(col < (int) cols.size());
  return cols[col];
}

void DataSet::AddObserver(std::function<void(const Index2 &)> &&fn) {
  observers.emplace_back(fn);
}

}