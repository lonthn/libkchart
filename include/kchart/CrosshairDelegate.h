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
