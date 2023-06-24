//
// Created by luo-zeqi on 2013/4/18.
//

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

typedef struct ColumnInfo {
  CStringW name;
  int precision;
  int index;
} *ColumnKey;

struct Index2 {
  int col;
  int row;
};

/// @brief ԭʼ���ݼ�, ��2ά������ʽ�洢ͼ�ζ�Ӧ��ԭʼ
/// �����ҿ���ʱ����.
/// @details ͨ�����ǻᴴ��һ��ͼ�λ����Զ���ͼ��, ����
/// ������ͼ����������ݶ�Ӧ�� ColumnKey(ͨ��AddCol/FindCol���).
struct DataSet {
public:
  DataSet();

  /// @brief �����, ���ṩ����.
  ColumnKey AddCol(const std::string &name, int precision = 1);

  /// @brief ���һ��.
  int AddRow();

  /// @brief �����, ��ָ������.
  void AddRow(int n);

  /// @brief ������������ ColumnKey(������, ���ڻ�ȡ
  /// ��Ӧ�е�����).
  ColumnKey FindCol(const std::string &name);
  inline int RowCount() const { return rowCount; }
  inline int ColCount() const { return (int) cols.size(); }

  DataType Get(int col, int row);
  DataType Get(const Index2 &idx);
  DataType Get(ColumnKey col, int row);
  void Set(int col, int row, DataType val);
  void Set(const Index2 &idx, DataType val);
  void Set(ColumnKey col, int row, DataType val);
  DataRows &Get(ColumnKey col);
  DataRows &operator[](int col);

  void AddObserver(std::function<void(const Index2 &)> &&fn);

private:
  DataCols cols;
  int rowCount;
  std::map<std::string, ColumnInfo> colKeys;
  std::vector<std::function<void(const Index2 &)>> observers;
};

/// @brief �ڻ�ͼʱ������������Ҫ������.
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

  /// @brief ��ȡ���ݵ�X��λ��, ���ڵ���ͼ�����Լ���ռ��,
  /// ����Ϊ�˷����ͼ, ���ؾ���λ��.
  inline Scalar ToPX(int idx) const {
    if (sWidth == 1) {
      return Scalar(wRatio * float(idx));
    }

    if ((size.width / sWidth) > count)
      return idx * sWidth + (sWidth / 2 + 1);

    return size.width - (count - idx) * sWidth + (sWidth / 2 + 1);
  }

  /// @brief ��ȡ���ݵ�Y��λ��
  inline Scalar ToPY(DataType val) const {
    return size.height - Scalar(hRatio * float(val - bias));
  }

  /// @brief ������Ļ�������ö�Ӧ��������
  inline int ToIdx(Scalar px) const {
    int index;
    int last = count - 1;
    if (sWidth == 1) {
      index = int(float(px) / wRatio);
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

  /// @brief ������Ļ�������������������
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
