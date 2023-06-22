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
   * ������Լ���������ϵ�����Ӧ����ʲô���ģ�Ĭ���ǽ�����
   * ����2λС��.
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
  // ����ͨ�������ж���������߻����ұ�.
  bool alignToRight_;

  Scalar width_;
  Color scaleColor_;
  Scalar crosshairY_;
  CStringW crosshairText_;
  // ʮ�ֳ߿̶����ֵĵ�ɫ.
  Color crosshairBackColor_;

  TransformFn transformFn_;

  std::vector<DataType> scales_;
  // ���ƿ̶�ʱ��ʹ�õĿɲ����������ͣ�Ϊ�˱����ظ�ת�������ǻ���һ��.
  std::vector<CStringW> strScales_;
};

static CStringW ToStringWithDigit(DataType val) {
  return DoubleToStr(val, 2);
}

// �������������չʾ�����ǿ���û���㹻�Ŀռ䣬���Կ���
// ���Դ��ϵ�λ.
static CStringW ToStringWithUnit(DataType val) {
  if (val < 1e4)
    return DoubleToStr(val, 2);
  else if (val < 1e8)
    return DoubleToStr(val / 1e4, 2) + L"��";
  else
    return DoubleToStr(val / 1e8, 2) + L"��";
}

}

#endif //LIBKCHART_VERTICALAXIS_H
