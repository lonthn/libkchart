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

#ifndef LIBKCHART_VERTICALAXIS_H
#define LIBKCHART_VERTICALAXIS_H

#include "graph/Scalar.h"
#include "graph/GraphContext.h"
#include "Def.h"
#include "Struct.h"
#include "StrUtils.h"

namespace kchart {

typedef CStringW (*TransformFn)(DataType, int, int);
static CStringW ToStrWithUnit(DataType val, int precision, int decimals);

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
   * ������Լ���������ϵ�����Ӧ����ʲô���ģ�Ĭ���ǽ�����
   * ����2λС��.
   * @param fn
   */
  void SetFormatter(TransformFn fn) { transformFn_ = fn; }
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
  // ����ͨ�������ж���������߻����ұ�.
  bool alignToRight_;

  Scalar width_;
  Color scaleColor_;
  Scalar crosshairY_;
  CStringW crosshairText_;
  // ʮ�ֳ߿̶����ֵĵ�ɫ.
  Color crosshairBackColor_;

  int precision_;
  int decimals_;
  TransformFn transformFn_;

  std::vector<DataType> scales_;
  // ���ƿ̶�ʱ��ʹ�õĿɲ����������ͣ�Ϊ�˱����ظ�ת�������ǻ���һ��.
  std::vector<CStringW> strScales_;
};

// �������������չʾ�����ǿ���û���㹻�Ŀռ䣬���Կ���
// ���Դ��ϵ�λ.
static CStringW ToStrWithUnit(
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
    unit = L"��";
    dval /= 10000;
  } else {
    unit = L"��";
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
