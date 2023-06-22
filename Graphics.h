//
// Created by luo-zeqi on 2013/4/17.
//

#ifndef LIBKCHART_GRAPHICS_H
#define LIBKCHART_GRAPHICS_H

#include "Def.h"
#include "Struct.h"
#include "graph/GraphContext.h"

namespace kchart {

class Graphics {
public:
  explicit Graphics(int colNum)
      : centralAxis(NAN)
      , Digit(2)
      , NormalColor(0xFFFFFFFF) {
    cids.resize(colNum, 0);
  }

  virtual ~Graphics() = default;

  /// 在图形被加入GraphArea后，GraphArea会在ColorList中
  /// 选取一个颜色进行设置, 当然我们也可以自定义颜色.
  virtual bool SetColor(Color color) {
    return false;
  }

  /// @brief 获取图形颜色.
  virtual Color GetColor(const DrawData &data, int i) {
    return NormalColor;
  }

  /// @brief 获取图形颜色, 带中心轴的情况下.
  virtual Color GetColorWithCA(const DrawData &data, int i) {
    return NormalColor;
  }

  /// 重写 Paint 以绘制需要的图形
  virtual void Paint(
      GraphContext *gctx,
      const DrawData &data
  ) = 0;

  std::vector<ColumnKey> cids;
  DataType centralAxis;
  int Digit;
  Color NormalColor;
};

/// K线图，需要提供开高低收4列数据
/// 列名分别是: OPEN, HIGH, LOW, CLOSE
class KLineGraph : public Graphics {
  enum Cid {
    Open = 0,
    High,
    Low,
    Close,
  };

public:
  Color UpColor;
  Color DownColor;
  Color TextColor;

  explicit KLineGraph(DataSet &data)
      : Graphics(4) {
    UpColor = 0xFFFF4A4A;
    DownColor = 0xFF54FCFC;
    TextColor = 0xFFFFFFFF;

    cids[Open] = data.FindCol("OPEN");
    cids[High] = data.FindCol("HIGH");
    cids[Low] = data.FindCol("LOW");
    cids[Close] = data.FindCol("CLOSE");
  }

  void Paint(
      GraphContext *gctx,
      const DrawData &data
  ) override;
};


class PolyLineGraph : public Graphics {
public:
  explicit PolyLineGraph(ColumnKey col)
      : Graphics(1) {
    cids[0] = col;
  }

  bool SetColor(Color color) override {
    NormalColor = color;
    return true;
  }

  void Paint(
      GraphContext *gctx,
      const DrawData &data
  ) override;
};

/// 柱状图
class HistogramGraph : public Graphics {
public:
  Color UpColor;
  Color DownColor;
  Scalar FixedWidth;

  explicit HistogramGraph(ColumnKey key)
      : Graphics(1) {
    UpColor = 0xFFFF4A4A;
    DownColor = 0xFF54FCFC;
    FixedWidth = -1;

    cids[0] = key;
  }

  Color GetColorWithCA(const DrawData &data, int i) override {
    DataType val = data.Get(cids[0], i);

    if (val > centralAxis)
      return UpColor;
    else if (val < centralAxis)
      return DownColor;

    return NormalColor;
  }

  void Paint(
      GraphContext *gctx,
      const DrawData &data
  ) override;
};

/// @brief 特殊柱状图, 需要额外2列数据(OPEN,CLOSE)来决定单根柱
/// 图颜色, 用于K线图中的成交量(额).
class VolumeGraph : public HistogramGraph {
public:
  explicit VolumeGraph(DataSet &data)
      : HistogramGraph(data.FindCol("VOLUME")) {
    openKey_ = data.FindCol("OPEN");
    closeKey_ = data.FindCol("CLOSE");
  }

  Color GetColor(const DrawData &data, int i) override {
    DataType open = data.Get(openKey_, i);
    DataType close = data.Get(closeKey_, i);

    if (open < close)
      return UpColor;
    else if (open > close)
      return DownColor;

    return NormalColor;
  }

private:
  ColumnKey openKey_;
  ColumnKey closeKey_;
};

}

#endif //LIBKCHART_GRAPHICS_H
