//
// Created by luo-zeqi on 2013/4/17.
//

#include <algorithm>
#include "Graphics.h"

namespace kchart {

void KLineGraph::Paint(GraphContext *gctx, const DrawData &data)
{
    Size areaSize = data.size;
    Scalar width = data.sWidth;
    Scalar margin = data.sMargin;
    Scalar reactWidth = width - margin * 2;
    if (width == reactWidth)
        width = 1;

    int minIdx = data.Count() - 1;
    int maxIdx = data.Count() - 1;
    DataType min = data.Get(cids[Low], minIdx);
    DataType max = data.Get(cids[High], maxIdx);

    // 反向绘制使得窗体缩放时不会导致图形抖动
    for (int i = data.Count() - 1; i >= 0; i--)
    {
        Scalar x = data.ToPX(i);

        DataType open  = data.Get(cids[Open], i);
        DataType high  = data.Get(cids[High], i);
        DataType low   = data.Get(cids[Low], i);
        DataType close = data.Get(cids[Close], i);

        if (min > low)
        {
            min = low;
            minIdx = i;
        }
        if (max < high)
        {
            max = high;
            maxIdx = i;
        }

        if (close > open)
            gctx->SetColor(UpColor);
        else if (close < open)
            gctx->SetColor(DownColor);
        else
            gctx->SetColor(NormalColor);

        // 影线
        if (width == 1 || open == close)
        {
            if (high != low)
            {
                Scalar top = data.ToPY(high);
                Scalar bottom = data.ToPY(low);
                gctx->DrawLine(x, top, x, bottom);
            }
        }
        else
        {
            DataType upStart, downStart;
            upStart   = std::max(open, close);
            downStart = std::min(open, close);
            if (upStart < high)
            {
                Scalar top = data.ToPY(high);
                Scalar bottom = data.ToPY(upStart);
                gctx->DrawLine(x, top, x, bottom);
            }
            if (downStart > low)
            {
                Scalar top = data.ToPY(downStart);
                Scalar bottom = data.ToPY(low);
                gctx->DrawLine(x, top, x, bottom);
            }
        }

        // 绘制箱体
        if (width > 1)
        {
            Scalar left = x - (reactWidth/2);
            if (open < close) // 阳线
            {
                gctx->FillRect(
                    left, data.ToPY(close),
                    left + reactWidth, data.ToPY(open)
                );
            }
            else if (open > close) // 阴线
            {
                gctx->FillRect(
                    left, data.ToPY(open),
                    left + reactWidth, data.ToPY(close)
                );
            }
            else // 平盘
            {
                Scalar y = data.ToPY(close);
                gctx->DrawLine(left, y, left + reactWidth, y);
            }
        }
    }

    // 最高与最低点标注
    gctx->SetColor(TextColor);

    auto fn = [=](DataType num, int i)
    {
        Scalar dis = 10;
        WStr str = WStr::ToString(num, Digit);
        Size size = gctx->MeasureStr(str);

        Scalar halfHeight = size.height/2;
        Scalar ny = data.ToPY(num);
        Scalar ty = ny;
        if ((areaSize.height - ty) < halfHeight) {
            ty -= halfHeight - (areaSize.height - ty);
        }
        ty -= halfHeight;
        if (ty < 0) {
            ty = 0;
        }

        Scalar x = data.ToPX(i);
        if (i < data.Count()/2) {
            gctx->DrawLine(x, ny, x + dis, ty + halfHeight);
            x += dis;
        } else {
            gctx->DrawLine(x, ny, x - dis, ty + halfHeight);
            x -= dis + size.width;
        }

        gctx->DrawStr(str, {x, ty});
    };

    fn(min, minIdx);
    fn(max, maxIdx);
}


void VolumeGraph::Paint(GraphContext *gctx, const DrawData &data)
{
    Scalar width = data.sWidth;
    Scalar reactWidth = width - data.sMargin*2;
    if (width == reactWidth)
        width = 1;

    Scalar x;

    // 反向绘制使得窗体缩放时不会抖动
    for (int i = data.Count() - 1; i >= 0; i--)
    {
        x = data.ToPX(i);

        DataType open  = data.Get(cids[0], i);
        DataType close = data.Get(cids[1], i);
        DataType vol   = data.Get(cids[2], i);

        if (close > open)
            gctx->SetColor(UpColor);
        else if (close < open)
            gctx->SetColor(DownColor);
        else
            gctx->SetColor(NormalColor);

        Scalar top = data.ToPY(vol);
        Scalar bottom = data.ToPY(0);
        Scalar left = x - (reactWidth/2);
        if (width == 1)
        {
            gctx->DrawLine(x, top, x, bottom);
        }
        else
        {
            gctx->FillRect(
                left, top,
                left + reactWidth, bottom
            );
//            if (open < close) // 红柱
//            {
//                gctx->FillRect(
//                    left, top,
//                    left + reactWidth, bottom
//                );
//            }
//            else if (open > close) // 绿柱
//            {
//                gctx->FillRect(
//                    left, top,
//                    left + reactWidth, bottom
//                );
//            }
//            else
//            {
//            }
        }
    }
}


void PolyLineGraph::Paint(GraphContext *gctx, const DrawData &data)
{
    int i = 0;
    for (; i < data.Count(); i++)
    {
        DataType val = data.Get(cids[0], i);
        if (!isnan(val))
            break;
    }

    gctx->SetColor(LineColor);

    gctx->BeginPolyLine(data.Count() - i);

    for (int j = data.Count() - 1; j >= i; j--)
    {
        DataType val = data.Get(cids[0], j);
        Scalar x = data.ToPX(j);
        Scalar y = data.ToPY(val);
        gctx->AddPolyLine({x, y});
    }

    gctx->EndPolyLine();
}

void HistogramGraph::Paint(GraphContext *gctx, const DrawData &data)
{
    bool validca = !isnan(centralAxis);
    DataType baseLine = validca ? centralAxis : 0;

    gctx->SetColor(NormalColor);

    // 反向绘制使得窗体缩放时不会抖动
    for (int i = data.Count() - 1; i >= 0; i--)
    {
        DataType val  = data.Get(cids[0], i);

        if (baseLine == val)
            continue;

        if (validca)
        {
            if (val > centralAxis)
                gctx->SetColor(UpColor);
            else if (val < centralAxis)
                gctx->SetColor(DownColor);
        }

        Scalar x = data.ToPX(i);
        Scalar top = data.ToPY(val);
        Scalar bottom = data.ToPY(baseLine);
        gctx->DrawLine(x, top, x, bottom);

//        else
//        {
//            if (open < close) // 红柱
//            {
//                gctx->DrawRect(
//                x, top,
//                x + reactWidth, bottom
//                );
//            }
//            else if (open > close) // 绿柱
//            {
//                gctx->FillRect(
//                x, top,
//                x + reactWidth, bottom
//                );
//            }
    }

}

}
