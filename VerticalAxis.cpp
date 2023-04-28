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

void VerticalAxis::OnPaint(GraphContext *ctx, DrawData& data,
                           Scalar height)
{
    ctx->SetColor(scaleColor_);

    Scalar pad = 5;

    for (int i = 0; i < (int) scales_.size(); i++)
    {
        Scalar y = data.ToPY(scales_[i]);

        Size size = ctx->MeasureStr(strScales_[i]);
        if (alignToRight_) {
            ctx->DrawStr(strScales_[i], {width_ - size.width - pad,
                                         y - (size.height / 2)});
        } else {
            ctx->DrawStr(strScales_[i], {pad, y - (size.height / 2)});
        }
    }
}

}