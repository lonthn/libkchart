//
// Created by luo-zeqi on 2013/4/17.
//

#include <algorithm>
#include "Graphics.h"

namespace kchart {

void KLineGraph::Paint(GraphContext *gctx, DrawData& data)
{
    Size areaSize = data.GetSize();

    float  wratio = data.GetWRatio();
    Scalar width = Scalar(wratio);
    if (width < 3)
        width = 1;

    // ��ȵ�1/4��Ϊ��߾�
    Scalar margin = width / 4;
    // ����K��ʵ�ʿ��
    Scalar reactWidth = width - margin * 2;

    Scalar x = areaSize.width;

    int minIdx = data.Count() - 1;
    int maxIdx = data.Count() - 1;
    DataType min = data.Get(minIdx, cids[Low]->index);
    DataType max = data.Get(maxIdx, cids[High]->index);

    // �������ʹ�ô�������ʱ���ᵼ��ͼ�ζ���
    for (int i = data.Count() - 1; i >= 0; i--)
    {
        x -= (Scalar(wratio) - margin);

        DataType open  = data.Get(i, cids[Open]->index);
        DataType high  = data.Get(i, cids[High]->index);
        DataType low   = data.Get(i, cids[Low]->index);
        DataType close = data.Get(i, cids[Close]->index);

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

        // Ӱ��
        Scalar center = x + (reactWidth/2);
        if (width == 1 || open == close)
        {
            if (high != low)
            {
                Scalar top = data.ToPY(high);
                Scalar bottom = data.ToPY(low);
                gctx->DrawLine(center, top, center, bottom);
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
                gctx->DrawLine(center, top, center, bottom);
            }
            if (downStart > low)
            {
                Scalar top = data.ToPY(downStart);
                Scalar bottom = data.ToPY(low);
                gctx->DrawLine(center, top, center, bottom);
            }
        }

        // ��������
        if (width > 1)
        {
            if (open < close) // ����
            {
                gctx->FillRect(
                    x, data.ToPY(close),
                    x + reactWidth, data.ToPY(open)
                );
            }
            else if (open > close) // ����
            {
                gctx->FillRect(
                    x, data.ToPY(open),
                    x + reactWidth, data.ToPY(close)
                );
            }
            else // ƽ��
            {
                Scalar y = data.ToPY(close);
                gctx->DrawLine(x, y, x + reactWidth, y);
            }
        }

        x -= margin;
    }


    // �������͵��ע
    gctx->SetColor(TextColor);

    auto fn = [&data, gctx, areaSize, wratio, reactWidth, this]
    (DataType num, int i)
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

        Scalar x = areaSize.width - (data.Count() - i) * Scalar(wratio);
        x += reactWidth / 2; // ����
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


void VolumeGraph::Paint(
        GraphContext *gctx,
        DrawData& data
)
{
    Size areaSize = data.GetSize();

    float  wratio = data.GetWRatio();
    Scalar width = Scalar(wratio);
    if (width < 3)
        width = 1;

    // ��ȵ�1/4��Ϊ��߾�
    Scalar margin = width / 4;
    // ��������ʵ�ʿ��
    Scalar reactWidth = width - margin*2;

    Scalar x = areaSize.width;

    // �������ʹ�ô�������ʱ���ᶶ��
    for (int i = data.Count() - 1; i >= 0; i--)
    {

        x -= (Scalar(wratio) - margin);

        DataType open  = data.Get(i, cids[0]->index);
        DataType close = data.Get(i, cids[1]->index);
        DataType vol  = data.Get(i,  cids[2]->index);

        if (close > open)
            gctx->SetColor(UpColor);
        else if (close < open)
            gctx->SetColor(DownColor);
        else
            gctx->SetColor(NormalColor);

        Scalar top = data.ToPY(vol);
        Scalar bottom = data.ToPY(0);
        Scalar center = x + (reactWidth/2);
        if (width == 1)
        {
            gctx->DrawLine(center, top, center, bottom);
        }
        else
        {
            if (open < close) // ����
            {
                gctx->FillRect(
                        x, top,
                        x + reactWidth, bottom
                );
            }
            else if (open > close) // ����
            {
                gctx->FillRect(
                        x, top,
                        x + reactWidth, bottom
                );
            }
            else
            {
            }
        }

        x -= margin;
    }
}


void PolyLineGraph::Paint(GraphContext *gctx, DrawData& data)
{
    Size areaSize = data.GetSize();
    float  wratio = data.GetWRatio();
    //Scalar width = Scalar(wratio);

    Scalar x = areaSize.width - Scalar(wratio/2);

    int i = 0;
    for (; i < data.Count(); i++)
    {
        DataType val = data.Get(i, cids[0]->index);
        if (!isnan(val))
            break;
    }

    gctx->SetColor(LineColor);

    gctx->BeginPolyLine(data.Count() - i);

    for (int j = data.Count() - 1; j >= i; j--)
    {
        DataType val = data.Get(j, cids[0]->index);
        Scalar y = data.ToPY(val);
        gctx->AddPolyLine({x, y});

        x -= Scalar(wratio);
    }

    gctx->EndPolyLine();
}

void HistogramGraph::Paint(
    GraphContext *gctx,
    DrawData& data
)
{
    Size areaSize = data.GetSize();

    float  wratio = data.GetWRatio();
    Scalar width = 1;

    // ��ȵ�1/4��Ϊ��߾�
    Scalar margin = Scalar(wratio) / 4;
    // ��������ʵ�ʿ��
    Scalar reactWidth = Scalar(wratio) - margin*2;

    Scalar x = areaSize.width;

    gctx->SetColor(NormalColor);

    bool validca = !isnan(centralAxis);
    DataType baseLine = validca ? centralAxis : 0;

    // �������ʹ�ô�������ʱ���ᶶ��
    for (int i = data.Count() - 1; i >= 0; i--)
    {
        x -= (Scalar(wratio) - margin);

        DataType val  = data.Get(i, cids[0]->index);

        if (baseLine == val)
        {
            x -= margin;
            continue;
        }

        if (validca)
        {
            if (val > centralAxis)
                gctx->SetColor(UpColor);
            else if (val < centralAxis)
                gctx->SetColor(DownColor);
        }

        Scalar top = data.ToPY(val);
        Scalar bottom = data.ToPY(baseLine);
        Scalar center = x + (reactWidth/2);
        //if (width == 1)
        {
            gctx->DrawLine(center, top, center, bottom);
        }
//        else
//        {
//            if (open < close) // ����
//            {
//                gctx->DrawRect(
//                x, top,
//                x + reactWidth, bottom
//                );
//            }
//            else if (open > close) // ����
//            {
//                gctx->FillRect(
//                x, top,
//                x + reactWidth, bottom
//                );
//            }

        x -= margin;
    }

}

}
