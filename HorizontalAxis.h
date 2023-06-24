//
// Created by luo-zeqi on 2023/5/13.
//

#ifndef LIBKCHART_HORIZONTALAXIS_H
#define LIBKCHART_HORIZONTALAXIS_H

#include "Def.h"
#include "Struct.h"
#include "Date.h"
#include "graph/Scalar.h"
#include "graph/GraphContext.h"

namespace kchart {

//typedef WStr (*TransformFn)(DataType);

class KChartWnd;

/// 我是一个横向刻度轴
class HorizontalAxis {
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

  virtual void OnFitIdx(
      int begin, int end
  );
  virtual void OnMoveCrosshair(Point point);
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
