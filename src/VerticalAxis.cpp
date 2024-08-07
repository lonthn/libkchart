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

#include "VerticalAxis.h"

namespace kchart {

void VerticalAxis::SetScaleColor(Color color) {
  scaleColor_ = color;
}

void VerticalAxis::SetCrosshairBackColor(Color color) {
  crosshairBackColor_ = color;
}

void VerticalAxis::OnSetScales(const DataRows &scales) {
  scales_.resize(scales.size());
  strScales_.resize(scales.size());

  for (int i = 0; i < (int) scales.size(); i++) {
    scales_[i] = scales[i];
    strScales_[i] = transformFn_(scales[i], precision_, decimals_);
  }
}

void VerticalAxis::OnMoveCrosshair(Point point) {
  crosshairY_ = point.y;
}

Scalar VerticalAxis::OnMeasureWidth(GraphContext *ctx) {
  Scalar width = 0;
  scaleSize_.resize(strScales_.size());
  for (int i = 0; i < strScales_.size(); i++) {
    Size size = ctx->MeasureStr(strScales_[i]);
    scaleSize_[i] = size;
    width = max(width, size.width);
  }
  return width;
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
    crosshairText_ = transformFn_(val, precision_, decimals_);

    draw(crosshairY_, crosshairText_, true);
  }

  ctx->Translate({0, -offY});
}

}