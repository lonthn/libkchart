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

#ifndef LIBKCHART_STRUCT_H
#define LIBKCHART_STRUCT_H

#include "Def.h"
#include "graph/Scalar.h"

#include <vector>
#include <map>
#include <string>
#include <functional>
#include <atlstr.h>


namespace kchart {

struct Setter {
  int fieldOffset;
  typedef void (*Function)(kchart::DataType *, void *);
  Function fn;

  Setter(int off, Function f)
  : fieldOffset(off)
  , fn(f) {
  }

  void operator () (kchart::DataType *d, void *s) const {
    fn(d, ((char *)s) + fieldOffset);
  }
};

#define MAKE_SETTER(s, m, f) \
  new Setter(offsetof(s, m), f)

// �����������Ϣ
typedef struct ColumnInfo {
  CStringW name;
  // ����ʹ�����ʹ洢������ͨ����������
  // �洢������.
  int precision;
  int index;

  Setter *setter;
} *ColumnKey;

struct Index2 {
  int col;
  int row;
};

// @brief ԭʼ���ݼ�, ��2ά������ʽ�洢ͼ�ζ�Ӧ��ԭʼ
// �����ҿ���ʱ����.
// @details ͨ�����ǻᴴ��һ��ͼ�λ����Զ���ͼ��, ����
// ������ͼ����������ݶ�Ӧ�� ColumnKey(ͨ��AddCol/FindCol���).
struct DataSet {

  typedef std::function<void(DataSet &data, int)> ObserverFn;

  DataSet();

  // @brief �����, ���ṩ����.
  ColumnKey AddCol(const std::string &name);
  ColumnKey AddCol(const std::string &name, int precision);
  ColumnKey AddCol(const std::string &name, Setter *setter);
  ColumnKey AddCol(const std::string &name, int precision, Setter *setter);
  // @brief ���һ��.
  int AddRow();
  // @brief �����, ��ָ������.
  void AddRow(int n);

  // @brief ������������ ColumnKey(������, ���ڻ�ȡ
  // ��Ӧ�е�����).
  ColumnKey FindCol(const std::string &name);

  inline int RowCount() const { return rowCount; }
  inline int ColCount() const { return (int) cols.size(); }
  inline int LastRow() const { return rowCount - 1; }
  inline int Empty() const { return rowCount == 0; }

  DataType Get(int col, int row);
  DataType Get(const Index2 &idx);
  DataType Get(ColumnKey col, int row);
  void Set(int col, int row, DataType val);
  void Set(const Index2 &idx, DataType val);
  void Set(ColumnKey col, int row, DataType val);
  DataRows &Get(ColumnKey col);
  // @note ͨ���ú�����õ�����������������д�벢
  // ���ᴥ���۲���.
  DataRows &operator[](int col);

  void FillRow(void *src, int idx) {
    for (auto &item : colKeys) {
      ColumnKey col = &item.second;
      if (col->setter)
        (*col->setter)(&(cols[col->index][idx]), src);
    }
  }

  // @brief ������ݼ���, ��Notify�����ú��ͨ��ObserverFn
  // ֪ͨ���й۲���, priority ԽСԽ���յ�֪ͨ.
  int AddObserver(int priority, ObserverFn &&fn);
  void RemoveObserver(int id);
  // �������� Observer ������������.
  void Notify();

private:
  DataCols cols;
  int rowCount;
  int oldRowCount;
  std::map<std::string, ColumnInfo> colKeys;
  int observerCounter;
  std::vector<std::map<int, ObserverFn>> dataObservers;
  std::map<int, ObserverFn> observers;
};

// @brief �ڻ�ͼʱ������������Ҫ������.
struct DrawData {
public:
  // ��ͼ�����С
  Size size;
  // ����ͼ��ռ��
  Scalar sWidth;
  // ����ͼ�εı߾�
  Scalar sMargin;

  float wRatio;
  float hRatio;
  DataType bias;

  DrawData(DataSet &data, int off, int count)
      : data(data), off(off), count(count) {
    size = {};
    wRatio = 0.0;
    hRatio = 0.0;
    bias = 0;
  }

  inline int Count() const { return count; }

  inline DataType Get(int col, int row) const {
    return data.Get(col, off + row);
  }
  inline DataType Get(ColumnKey col, int row) const {
    return data.Get(col, off + row);
  }
  inline DataType Get(const Index2 &idx) const {
    return data.Get(idx.col, off + idx.row);
  }
  inline bool Empty() const {
    return count == 0;
  }
  inline int NativeIdx(int idx) const {
    return idx - off;
  }

  // @brief ��ȡ���ݵ�X��λ��, ���ڵ���ͼ�����Լ���ռ��,
  // ����Ϊ�˷����ͼ, ���ؾ���λ��.
  inline Scalar ToPX(int idx) const {
    if (sWidth == 1) {
      return Scalar(wRatio * float(idx));
    }

    if ((size.width / sWidth) > count)
      return idx * sWidth + (sWidth / 2 + 1);

    return size.width - (count - idx) * sWidth + (sWidth / 2 + 1);
  }

  // @brief ��ȡ���ݵ�Y��λ��
  inline Scalar ToPY(DataType val) const {
    return size.height - Scalar(hRatio * float(val - bias));
  }

  // @brief ������Ļ�������ö�Ӧ��������
  inline int ToIdx(Scalar px) const {
    int index;
    int last = count - 1;
    if (sWidth == 1) {
      index = int(floor(float(px) / wRatio));
    } else {
      // һ����˵, else �ķ������ܵõ�����, ��Ҫ���ǵ�ͼ��δ
      // ��������ʱ�Ǵ���߿�ʼ��ͼ��.
      if ((size.width / sWidth) > count) {
        index = px / sWidth;
      } else {
        index = ((size.width - px) / sWidth);
        index = last - index;
      }
    }
    if (index < 0)
      return 0;
    if (index > last)
      return last;
    return index;
  }

  // @brief ������Ļ�������������������
  inline DataType ToData(Scalar py) const {
    // py = size.height - Scalar(hRatio * float(val - bias));
    return DataType(float(size.height - py) / hRatio + float(bias));
  }

private:
  DataSet &data;
  int off;
  int count;
};

}

#endif //LIBKCHART_STRUCT_H
