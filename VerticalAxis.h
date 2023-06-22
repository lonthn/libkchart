//
// Created by luo-zeqi on 2013/4/21.
//

#ifndef LIBKCHART_VERTICALAXIS_H
#define LIBKCHART_VERTICALAXIS_H

#include "graph/Scalar.h"
#include "graph/GraphContext.h"
#include "Def.h"
#include "Struct.h"
#include "StrUtils.h"

namespace kchart {

typedef CStringW (*TransformFn)(DataType);
static CStringW ToStringWithDigit(DataType val);
static CStringW ToStringWithUnit(DataType val);

/**
 *
 */
class VerticalAxis {
public:
  explicit VerticalAxis(Scalar width, bool aRight)
      : width_(width)
      , scaleColor_(0xFF8F8F8F)
      , alignToRight_(aRight)
      , transformFn_(ToStringWithDigit)
      , crosshairY_(-1)
      , crosshairBackColor_(0xFF3F3F3F) {
  }

  virtual ~VerticalAxis() = default;

  /**
   * 你可以自己定义界面上的数据应该是什么样的，默认是将数据
   * 保留2位小数.
   * @param fn
   */
  void SetScaleFormatter(TransformFn fn) { transformFn_ = fn; }

  Scalar GetWidth() const { return width_; }

  virtual void OnSetScales(const DataRows &scales);
  virtual void OnMoveCrosshair(Point point);
  virtual void OnPaint(
      GraphContext *ctx,
      DrawData &data,
      Scalar offY
  );

private:
  // 可以通过它来判断我是在左边还是右边.
  bool alignToRight_;

  Scalar width_;
  Color scaleColor_;
  Scalar crosshairY_;
  CStringW crosshairText_;
  // 十字尺刻度文字的底色.
  Color crosshairBackColor_;

  TransformFn transformFn_;

  std::vector<DataType> scales_;
  // 绘制刻度时所使用的可不是数字类型，为了避免重复转换，我们缓存一下.
  std::vector<CStringW> strScales_;
};

static CStringW ToStringWithDigit(DataType val) {
  return DoubleToStr(val, 2);
}

// 如果将数据完整展示，我们可能没有足够的空间，所以可以
// 尝试带上单位.
static CStringW ToStringWithUnit(DataType val) {
  if (val < 1e4)
    return DoubleToStr(val, 2);
  else if (val < 1e8)
    return DoubleToStr(val / 1e4, 2) + L"万";
  else
    return DoubleToStr(val / 1e8, 2) + L"亿";
}

}

#endif //LIBKCHART_VERTICALAXIS_H
