//
// Created by luo-zeqi on 2023/6/27.
//

#include "CrosshairDelegate.h"

namespace kchart {

CrosshairDelegate::CrosshairDelegate()
    : crosshairIndex_(-1)
    , mstrCrosshairIdx_(-1)
    , crosshairPoint_{-1, -1}
    , forceChange_(false) {
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
