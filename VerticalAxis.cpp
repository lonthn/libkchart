//
// Created by luo-zeqi on 2013/4/21.
//

#include "VerticalAxis.h"

namespace kchart {

void VerticalAxis::OnSetScales(const DataRows &scales)
{
    scales_.resize(scales.size());
    strScales_.resize(scales.size());

    for (int i = 0; i < (int) scales.size(); i++)
    {
        scales_[i] = scales[i];
        strScales_[i] = transformFn_(scales[i]);
    }
}

void VerticalAxis::OnPaint(
    GraphContext *ctx,
    DrawData& data,
    Scalar offY
)
{
    ctx->SetColor(scaleColor_);
    ctx->SetFont(FontId_WRYH, 10);

    Scalar pad = 5;

    ctx->Translate({0, offY});

    for (int i = 0; i < (int) scales_.size(); i++)
    {
        Scalar y = data.ToPY(scales_[i]);

        Size size = ctx->MeasureStr(strScales_[i]);
        Scalar x = pad;
        if (alignToRight_) {
            x = width_ - size.width - pad;
        }

        ctx->DrawStr(strScales_[i], {x, y - (size.height / 2)});
    }

    ctx->Translate({0, -offY});
}

}