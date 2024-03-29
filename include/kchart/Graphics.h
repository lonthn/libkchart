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

#ifndef LIBKCHART_GRAPHICS_H
#define LIBKCHART_GRAPHICS_H

#include "Def.h"
#include "Struct.h"
#include "graph/GraphContext.h"

namespace kchart {

class Graphics {
public:
  explicit Graphics(int colNum)
      : centralAxis(KC_INVALID_DATA)
      , Digit(2)
      , NormalColor(0xFFF0F0F0)
      , ZeroOrigin(false) {
    cids.resize(colNum, 0);
  }

  virtual ~Graphics() = default;

  virtual void ChangeTheme(bool white) {
    NormalColor = white ? 0xFF101010 : 0xFFF0F0F0;
  }

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

  bool  ZeroOrigin;
  int   Digit;
  Color NormalColor;
};

/// 组合图形,
class CombGraph : public Graphics {
public:
  CombGraph(): Graphics(0) { }

public:
  std::vector<Graphics *> graph_;
};

/// K线图，需要提供开高低收4列数据
/// 列名分别是: OPEN, HIGH, LOW, CLOSE
class KLineGraph : public Graphics {

  enum Cid { Open = 0, High, Low, Close, };

public:
  Color UpColor;
  Color DownColor;
  Color TextColor;

  explicit KLineGraph(DataSet &data)
      : Graphics(4) {
    UpColor   = 0xFFFF4A4A;
    DownColor = 0xFF54FCFC;
    TextColor = NormalColor;

    cids[Open]  = data.FindCol("OPEN");
    cids[High]  = data.FindCol("HIGH");
    cids[Low]   = data.FindCol("LOW");
    cids[Close] = data.FindCol("CLOSE");
  }

  void ChangeTheme(bool white) override {
    Graphics::ChangeTheme(white);
    TextColor = NormalColor;
    DownColor = white ? 0xFF10AB62 : 0xFF54FCFC;
  }

  void Paint(
      GraphContext *gctx,
      const DrawData &data
  ) override;
};


class PolyLineGraph : public Graphics {
public:
  int LineWidth;

  explicit PolyLineGraph(ColumnKey col, int lw = 1)
      : Graphics(1)
      , LineWidth(lw) {
    cids[0] = col;
  }

  // 不需要切换
  void ChangeTheme(bool) override {}

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
  Color  UpColor;
  Color  DownColor;
  Scalar FixedWidth;

  explicit HistogramGraph(ColumnKey key, Scalar fw = -1)
      : Graphics(1)
      , FixedWidth(fw) {
    ZeroOrigin = true;
    UpColor = 0xFFFF4A4A;
    DownColor = 0xFF54FCFC;

    cids[0] = key;
  }

  void ChangeTheme(bool white) override {
    Graphics::ChangeTheme(white);
    DownColor = white ? 0xFF10AB62 : 0xFF54FCFC;
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
