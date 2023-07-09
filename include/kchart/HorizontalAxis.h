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

#ifndef LIBKCHART_HORIZONTALAXIS_H
#define LIBKCHART_HORIZONTALAXIS_H

#include "Def.h"
#include "Struct.h"
#include "CrosshairDelegate.h"
#include "graph/Scalar.h"
#include "graph/GraphContext.h"

namespace kchart {

//typedef WStr (*TransformFn)(DataType);

class KChartWnd;

/// 我是一个横向刻度轴
class HorizontalAxis : public CrosshairDelegate {
public:
  explicit HorizontalAxis(KChartWnd *wnd);

//  void SetScaleFormatter(TransformFn fn) {
//    transformFn_ = fn;
//  }
  Scalar GetHeight() const {
    return height_;
  }
  /// @brief 设置刻度值所使用的列.
  void SetScaleCKey(ColumnKey ckey) {
    hdKey_ = ckey;
  }
  void SetScaleColor(Color color);
  void SetCrosshairBackColor(Color color);

  void OnCrosshairIdxChanged(GraphContext *ctx, DrawData& data) override;

  virtual void OnFitIdx(
      int begin, int end
  );
  virtual void OnPaint(
      GraphContext *ctx,
      DrawData &data,
      Scalar width
  );

private:
  KChartWnd *master_;
  // 横轴以什么作为刻度值.
  ColumnKey hdKey_;

  Scalar height_;
  Color scaleColor_;
  Scalar crosshairX_;
  CStringW crosshairText_;
  Color crosshairBackColor_;

//  TransformFn transformFn_;

  std::vector<int> scales_;
  std::vector<CStringW> strScales_;
};

//WStr FormatDate(DataType val) {
//  int y, m, d;
//  GetDate(val, y, m, d);
//  wchar_t txt[32];
//  swprintf(txt, 31, L"%04d%02d%02d",
//           y, m, d);
//  return txt;
//}

}

#endif //LIBKCHART_HORIZONTALAXIS_H
