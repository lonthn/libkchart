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

/// @brief 原始数据集, 以2维表格的形式存储图形对应的原始
/// 数据且可随时扩容.
/// @details 通常我们会创建一个图形或者自定义图形, 并提
/// 供根据图形所需的数据对应的 ColumnKey(通过AddCol/FindCol获得).
struct DataSet {
public:
  DataSet();

  /// @brief 添加列, 需提供列名.
  ColumnKey AddCol(const std::string &name, int precision = 1);

  /// @brief 添加一行.
  int AddRow();

  /// @brief 添加行, 可指定行数.
  void AddRow(int n);

  /// @brief 根据列名查找 ColumnKey(类索引, 用于获取
  /// 对应列的数据).
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

/// @brief 在绘图时，这里有你需要的数据.
struct DrawData {
public:
  // 绘图区域大小
  Size size;
  // 单个图形占宽
  Scalar sWidth;
  // 单个图形的边距
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

  /// @brief 获取数据的X轴位置, 由于单个图形有自己的占宽,
  /// 所以为了方便绘图, 返回居中位置.
  inline Scalar ToPX(int idx) const {
    if (sWidth == 1) {
      return Scalar(wRatio * float(idx));
    }

    if ((size.width / sWidth) > count)
      return idx * sWidth + (sWidth / 2 + 1);

    return size.width - (count - idx) * sWidth + (sWidth / 2 + 1);
  }

  /// @brief 获取数据的Y轴位置
  inline Scalar ToPY(DataType val) const {
    return size.height - Scalar(hRatio * float(val - bias));
  }

  /// @brief 根据屏幕横坐标获得对应数据索引
  inline int ToIdx(Scalar px) const {
    int index;
    int last = count - 1;
    if (sWidth == 1) {
      index = int(float(px) / wRatio);
    } else {
      // 一般来说, else 的方案就能得到索引, 但要考虑当图形未
      // 充满界面时是从左边开始绘图的.
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

  /// @brief 根据屏幕纵坐标轴算出具体数据
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
