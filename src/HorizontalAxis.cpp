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

#include "HorizontalAxis.h"
#include "KChartWnd.h"

namespace kchart {

HorizontalAxis::HorizontalAxis(KChartWnd *wnd)
    : master_(wnd)
    , height_(25)
    , scaleColor_(0xFF8F8F8F)
    , hdKey_(nullptr)
    , crosshairX_(-1)
    , crosshairBackColor_(0xFF3F3F3F) {
}

void HorizontalAxis::SetScaleColor(Color color) {
  scaleColor_ = color;
}

void HorizontalAxis::SetCrosshairBackColor(Color color) {
  crosshairBackColor_ = color;
}

void HorizontalAxis::OnCrosshairIdxChanged(GraphContext *ctx, DrawData& data) {
  if (crosshairIndex_ != -1) {
    DataType val = crosshairIndex_;
    if (hdKey_)
      val = data.Get(hdKey_, (int)val);
    crosshairText_.Format(L"%lld", val);
  }
}

void HorizontalAxis::OnFitIdx(
    int begin, int end
) {
  DataSet &data = master_->DataRef();
  end = min(data.RowCount(), end);
  //end

  if (end - begin == 0)
    return;

  scales_.clear();
  strScales_.clear();

  scales_.push_back(begin);
  scales_.push_back(end - 1);

  // 用户如果不提供刻度值数据列，就直接将索引展示.
  CStringW a, b;
  if (hdKey_ == nullptr) {
    a.Format(L"%d", begin);
    b.Format(L"%d", end - 1);
  } else {
    a.Format(L"%lld", data.Get(hdKey_, begin));
    b.Format(L"%lld", data.Get(hdKey_, end - 1));
  }
  strScales_.push_back(a);
  strScales_.push_back(b);
}

void HorizontalAxis::OnPaint(
    GraphContext *ctx,
    DrawData &data,
    Scalar width
) {
  OnPrePaint(ctx, data);

  ctx->SetFont(FontId_WRYH, 10);

  Scalar pad = 0;
  auto draw = [&](Scalar x, CStringW &str, bool backFill) {
    Size size = ctx->MeasureStr(str);
    Scalar left = x - (size.width / 2);
    Scalar top = 3;
    if (left < 0)
      left = 0;
    if ((left + size.width) > width)
      left = width - size.width;

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
    Scalar x = data.ToPX(data.NativeIdx(scales_[i]));
    draw(x, strScales_[i], false);
  }

  if (crosshairIndex_ != -1) {
    draw(crosshairPoint_.x, crosshairText_, true);
  }
}

}