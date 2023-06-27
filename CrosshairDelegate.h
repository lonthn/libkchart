//
// Created by luo-zeqi on 2023/6/27.
//

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
