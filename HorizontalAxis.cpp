//
// Created by luo-zeqi on 2023/5/13.
//

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
    int val = crosshairIndex_;
    if (hdKey_)
      val = (int) data.Get(hdKey_, val);
    crosshairText_.Format(L"%d", val);
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
    a.Format(L"%d", (int) data.Get(hdKey_, begin));
    b.Format(L"%d", (int) data.Get(hdKey_, end - 1));
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