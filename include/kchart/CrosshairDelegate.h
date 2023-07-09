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

#ifndef LIBKCHART_CROSSHAIRDELEGATE_H
#define LIBKCHART_CROSSHAIRDELEGATE_H

#include "graph/Scalar.h"
#include "Struct.h"
#include "graph/GraphContext.h"

namespace kchart {

class CrosshairDelegate {
public:
  CrosshairDelegate();
  ~CrosshairDelegate() = default;

  Point GetCrosshairPoint() const;
  int GetCrosshairIndex() const;

  void SetForceChange();

  virtual void OnMoveCrosshair(Point point);
  // ��������ϵͳʹ�õ����ͣ���ᵼ����ͼ�α���Сʱͨ�� Point
  // �޷�׼ȷ��λ��ͼ������, �����ṩһ������ֱ�����������Ľӿ�.
  virtual void OnMoveCrosshair(int idx);

  virtual void OnPrePaint(GraphContext *ctx, DrawData& data);

  virtual void OnCrosshairIdxChanged(GraphContext *ctx, DrawData& data) { }

protected:
  // ��������-1ʱ,ʮ�ֹ��Ķ�λ���Դ�Ϊ׼, ��������ֵ��߼�����Ϊ��
  // ���������: 10 != data.ToPX(data.ToIdx(10))
  int mstrCrosshairIdx_;

  bool forceChange_;

  Point crosshairPoint_;
  int crosshairIndex_;
};

} // namespace kchart

#endif //LIBKCHART_CROSSHAIRDELEGATE_H
