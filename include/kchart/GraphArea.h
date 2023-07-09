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

#ifndef LIBKCHART_GRAPHAREA_H
#define LIBKCHART_GRAPHAREA_H

#include "Def.h"
#include "Struct.h"
#include "graph/GraphContext.h"
#include "Graphics.h"
#include "CrosshairDelegate.h"

#include <atlstr.h>

namespace kchart {

class KChartWnd;
class VerticalAxis;

/**
 * 图形区域，以上下布局的方式展示在 KCharWnd 中.
 */
class GraphArea : public CrosshairDelegate {
public:
  virtual ~GraphArea();

  virtual VerticalAxis *GetLeftAxis() {
    return lAxis_;
  }
  virtual VerticalAxis *GetRightAxis() {
    return rAxis_;
  }

  DataType GetCentralAxis();
  /**
   * 设置一个中心轴
   * @param n NAN 表示取消设置.
   */
  void SetCentralAxis(DataType n);
  /**
   * 是否需要强调中心轴的展示.
   * @param flag
   */
  void SetBoldCentralAxis(bool flag);
  void SetZeroOrigin(bool flag);
  void SetDecimals(int decimals);
  void SetLabelVisible(bool flag);
  void SetLabelBackColor(Color color);
  void SetScaleLineColor(Color color);

  bool AddGraphics(Graphics *graph);
  bool AddGraphics(const std::vector<Graphics *> &graph);

protected:
  explicit GraphArea(KChartWnd *panel,
                     VerticalAxis *la,
                     VerticalAxis *ra);

  void SetWeight(float weight);
  float GetWeight() const;
  void SetBounds(const Rect &bounds);
  Rect GetBounds() const;
  Scalar GetLabelHeight() const;
  // 代表当前界面展示的所有数据的 min max
  DataType GetMin() const;
  DataType GetMax() const;
  const DataRows &GetScales();

  Scalar GetContentTop() const;
  Scalar GetContentHeight() const;
  void ReGatherLabel(GraphContext *ctx, DrawData &data);

  virtual void UpdateMinMax();
  virtual void UpdateScales();

  void OnCrosshairIdxChanged(GraphContext *ctx, DrawData& data) override;

  virtual void OnFitIdx(int begin, int end);
  virtual void OnPaint(GraphContext *ctx, DrawData &data);
  virtual void OnPaintLabel(GraphContext *ctx, DrawData &data);
  virtual void OnPaintGraph(GraphContext *ctx, DrawData &data);
  virtual void OnPaintCrosshair(GraphContext *ctx, DrawData &data);

protected:
  KChartWnd *panel_;
  float weight_;

  Rect bounds_;
  Rect graphArea_;

  Scalar labelHeight_;
  bool labelVisible_;
  Color labelBackColor_;

  /// 十字线所定位的数据可以通过标签展示.
  struct Label {
    CStringW text;
    Color color;
    Size size;
  };
  std::vector<Label> labels_;

  int begin_;
  int end_;
  int cacheMinIdx_;
  DataType cacheMin_;
  int cacheMaxIdx_;
  DataType cacheMax_;
  bool validCache_;
  int decimals_;

  Color scaleLineColor_;
  int colorIdx_;
  // 内置颜色列表, 供图形使用.
  std::vector<Color> colorList_;
  DataRows scales_;
  VerticalAxis *lAxis_;
  VerticalAxis *rAxis_;
  DataType centralAxis_;
  bool boldCA_;
  bool zeroOrigin_;

  std::vector<Graphics *> graphics_;
  std::vector<ColumnKey>  columns_;

  friend KChartWnd;
  friend VerticalAxis;
};

}

#endif //LIBKCHART_GRAPHAREA_H
