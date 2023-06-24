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

typedef CStringW (*TransformFn)(DataType, int, int);
static CStringW ToStringWithUnit(DataType, int, int);

/**
 *
 */
class VerticalAxis {
public:
  explicit VerticalAxis(Scalar width, bool aRight)
      : width_(width)
      , scaleColor_(0xFF8F8F8F)
      , alignToRight_(aRight)
      , precision_(1)
      , decimals_(2)
      , transformFn_(DataToStr)
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
  void SetPrecision(int n) { precision_ = n; }
  void SetDecimals(int d) { decimals_ = d; }
  void SetScaleColor(Color color);
  void SetCrosshairBackColor(Color color);

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

  int precision_;
  int decimals_;
  TransformFn transformFn_;

  std::vector<DataType> scales_;
  // 绘制刻度时所使用的可不是数字类型，为了避免重复转换，我们缓存一下.
  std::vector<CStringW> strScales_;
};

// 如果将数据完整展示，我们可能没有足够的空间，所以可以
// 尝试带上单位.
static CStringW ToStringWithUnit(
    DataType val,
    int precision,
    int decimals
) {
  int64_t num = val / precision;
  if (num < 10000)
    return DataToStr(val, precision, decimals);

  double dval = double(val)/double(precision);
  CStringW unit;
  if (num < 100000000) {
    unit = L"万";
    dval /= 10000;
  } else {
    unit = L"亿";
    dval /= 100000000;
  }

#define FORMAT_OUT_WITH_UNIT(format_str) \
  str.Format(L##format_str, dval, unit.GetString())

  CStringW str;
  switch (decimals) {
    case 0: str.Format(L"%lld%s", val/precision, unit.GetString()); break;
    case 1: FORMAT_OUT_WITH_UNIT("%.01f%s"); break;
    case 2: FORMAT_OUT_WITH_UNIT("%.02f%s"); break;
    case 3: FORMAT_OUT_WITH_UNIT("%.03f%s"); break;
    case 4: FORMAT_OUT_WITH_UNIT("%.04f%s"); break;
    case 5: FORMAT_OUT_WITH_UNIT("%.05f%s"); break;
    case 6: FORMAT_OUT_WITH_UNIT("%.06f%s"); break;
    case 7: FORMAT_OUT_WITH_UNIT("%.07f%s"); break;
    case 8: FORMAT_OUT_WITH_UNIT("%.08f%s"); break;
    case 9: FORMAT_OUT_WITH_UNIT("%.09f%s"); break;
    default: str.Format(L"%lld", val/precision, unit.GetString());
  }

  return str;
}

}

#endif //LIBKCHART_VERTICALAXIS_H
