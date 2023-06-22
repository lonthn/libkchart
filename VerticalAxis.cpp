//
// Created by luo-zeqi on 2013/4/21.
//

#include "VerticalAxis.h"

namespace kchart {

void VerticalAxis::OnSetScales(const DataRows &scales) {
  scales_.resize(scales.size());
  strScales_.resize(scales.size());

  for (int i = 0; i < (int) scales.size(); i++) {
    scales_[i] = scales[i];
    strScales_[i] = transformFn_(scales[i]);
  }
}

void VerticalAxis::OnMoveCrosshair(Point point) {
  crosshairY_ = point.y;
}

void VerticalAxis::OnPaint(
    GraphContext *ctx,
    DrawData &data,
    Scalar offY
) {
  ctx->SetFont(FontId_WRYH, 10);

  Scalar pad = 5;

  ctx->Translate({0, offY});

  auto draw = [&](Scalar y, CStringW &str, bool backFill) {
    Size size = ctx->MeasureStr(str);
    Scalar left = pad;
    Scalar top = y - (size.height / 2);
    if (alignToRight_) {
      left = width_ - size.width - pad;
    }

    if (backFill) {
      ctx->SetColor(crosshairBackColor_);
      ctx->FillRect(left, top,
                    left + size.width,
                    top + size.height);
    }

    ctx->SetColor(scaleColor_);
    ctx->DrawStr(str, {left, top});
  };

  for (int i = 0; i < (int) scales_.size(); i++) {
    Scalar y = data.ToPY(scales_[i]);
    draw(y, strScales_[i], false);
  }

  if (crosshairY_ != -1) {
    DataType val = data.ToData(crosshairY_);
    crosshairText_ = transformFn_(val);

    draw(crosshairY_, crosshairText_, true);
  }

  ctx->Translate({0, -offY});
}

}