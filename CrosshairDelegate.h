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
  // 由于坐标系统使用的整型，这会导致在图形被缩小时通过 Point
  // 无法准确定位到图形索引, 所以提供一个可以直接设置索引的接口.
  virtual void OnMoveCrosshair(int idx);

  virtual void OnPrePaint(GraphContext *ctx, DrawData& data);

  virtual void OnCrosshairIdxChanged(GraphContext *ctx, DrawData& data) { }

protected:
  // 在他大于-1时,十字光标的定位就以此为准, 有这种奇怪的逻辑是因为存
  // 在这种情况: 10 != data.ToPX(data.ToIdx(10))
  int mstrCrosshairIdx_;

  bool forceChange_;

  Point crosshairPoint_;
  int crosshairIndex_;
};

} // namespace kchart

#endif //LIBKCHART_CROSSHAIRDELEGATE_H
