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

#include "CrosshairDelegate.h"

namespace kchart {

CrosshairDelegate::CrosshairDelegate()
    : crosshairIndex_(-1)
    , mstrCrosshairIdx_(-1)
    , forceChange_(false) {
  crosshairPoint_ = {-1, -1};
}

Point CrosshairDelegate::GetCrosshairPoint() const {
  return crosshairPoint_;
}

int CrosshairDelegate::GetCrosshairIndex() const {
  return crosshairIndex_;
}

void CrosshairDelegate::SetForceChange() {
  forceChange_ = true;
}

void CrosshairDelegate::OnMoveCrosshair(Point point) {
  mstrCrosshairIdx_ = -1;
  crosshairPoint_ = point;
}

void CrosshairDelegate::OnMoveCrosshair(int idx) {
  mstrCrosshairIdx_ = idx;
}

void CrosshairDelegate::OnPrePaint(GraphContext *ctx, DrawData &data) {
  int index = -1;
  if (mstrCrosshairIdx_ < 0) {
    if (crosshairPoint_.x != -1)
      index = data.ToIdx(crosshairPoint_.x);
    if (forceChange_ || crosshairIndex_ != index) {
      crosshairIndex_ = index;
      OnCrosshairIdxChanged(ctx, data);
    }
  } else {
    if (forceChange_ || mstrCrosshairIdx_ != crosshairIndex_) {
      crosshairIndex_ = mstrCrosshairIdx_;
      crosshairPoint_.x = data.ToPX(crosshairIndex_);
      OnCrosshairIdxChanged(ctx, data);
    }
  }

  forceChange_ = false;
}

}
