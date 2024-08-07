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

#include "Struct.h"

#include <atlconv.h>

namespace kchart {

DataSet::DataSet()
    : rowCount(0)
    , oldRowCount(0) {
}

ColumnKey DataSet::CreateCol(const std::string &name) {
  if (name.empty())
    return nullptr;

  auto iter = columns.find(name);
  if (iter != columns.end())
    return &iter->second;

  USES_CONVERSION;
  CStringW wname = A2W(name.c_str());
  columns.emplace(name, ColumnInfo{wname, 1, ColCount()});
  raw.emplace_back(rowCount);

  return &columns[name];
}

ColumnKey DataSet::CreateCol(const std::string &name, int precision) {
  ColumnKey col = CreateCol(name);
  if (col) col->precision = precision;
  return col;
}

ColumnKey DataSet::CreateCol(const std::string &name, Setter *setter) {
  ColumnKey col = CreateCol(name);
  if (col) col->setter = setter;
  return col;
}

ColumnKey DataSet::CreateCol(const std::string &name, int precision, Setter *setter) {
  ColumnKey col = CreateCol(name);
  if (col) {
    col->precision = precision;
    col->setter = setter;
  }
  return col;
}

int DataSet::AddRow() {
  for (auto &col: raw) {
    col.resize(col.size() + 1);
  }

  return rowCount++;
}

void DataSet::AddRow(int n) {
  for (auto &col: raw) {
    col.resize(col.size() + n);
  }

  rowCount += n;
}

ColumnKey DataSet::FindCol(const std::string &name) {
  auto iter = columns.find(name);
  if (iter != columns.end())
    return &iter->second;

  return nullptr;
}

DataType DataSet::Get(int col, int row) {
  assert(col < (int) raw.size());
  assert(row < rowCount);
  return raw[col][row];
}

DataType DataSet::Get(const Index2 &idx) {
  assert(idx.col < (int) raw.size());
  assert(idx.row < rowCount);
  return raw[idx.col][idx.row];
}

DataType DataSet::Get(ColumnKey col, int row) {
  assert(col->index < (int) raw.size());
  assert(row < rowCount);
  return raw[col->index][row];
}

void DataSet::Set(int col, int row, DataType val) {
  assert(col < (int) raw.size());
  assert(row < rowCount);
  raw[col][row] = val;

//  for (auto &item: dataObservers[col])
//    item.second({col, row});
}

void DataSet::Set(const Index2 &idx, DataType val) {
  assert(idx.col < (int) raw.size());
  assert(idx.row < rowCount);
  raw[idx.col][idx.row] = val;

//  for (auto &item: dataObservers[idx.col])
//    item.second(idx);
}

void DataSet::Set(ColumnKey col, int row, DataType val) {
  assert(col->index < (int) raw.size());
  assert(row < rowCount);
  raw[col->index][row] = val;

//  for (auto &item: dataObservers[col->index])
//    item.second({col->index, row});
}

DataRows &DataSet::Get(ColumnKey col) {
  assert(col->index < (int) raw.size());
  return raw[col->index];
}

DataRows &DataSet::operator[](int col) {
  assert(col < (int) raw.size());
  return raw[col];
}


int DataSet::AddObserver(int priority, ObserverFn &&fn) {
  int id = (observerCounter++) + priority;
  observers.emplace(id, fn);
  return id;
}
void DataSet::RemoveObserver(int id) {
  observers.erase(id);
}

void DataSet::Notify() {
  for (auto &item : observers)
    item.second(*this, oldRowCount);
  oldRowCount = rowCount;
}

}