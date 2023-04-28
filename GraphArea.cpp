//
// Created by luo-zeqi on 2013/4/18.
//

#include "GraphArea.h"

#include "KChartWnd.h"

namespace kchart {

GraphArea::~GraphArea()
{
}

GraphArea::GraphArea(KChartWnd *panel,
                     VerticalAxis *la,
                     VerticalAxis *ra)
: panel_(panel)
, validCache_(false)
, lAxis_(la)
, rAxis_(ra)
, scaleLineColor_(0xFF303030)
, colorIdx_(0)
, centralAxis_(NAN)
{
   colorList_.emplace_back(0xFF3cb44b);
   colorList_.emplace_back(0xFFffe119);
   colorList_.emplace_back(0xFF4363d8);
   colorList_.emplace_back(0xFFf58231);
   colorList_.emplace_back(0xFF911eb4);
   colorList_.emplace_back(0xFF42d4f4);
   colorList_.emplace_back(0xFFf032e6);
   colorList_.emplace_back(0xFFbfef45);
   colorList_.emplace_back(0xFFfabed4);
   colorList_.emplace_back(0xFF469990);
   colorList_.emplace_back(0xFFdcbeff);
   colorList_.emplace_back(0xFF9A6324);
   colorList_.emplace_back(0xFFfffac8);
   colorList_.emplace_back(0xFF800000);
   colorList_.emplace_back(0xFFaaffc3);
   colorList_.emplace_back(0xFF808000);
   colorList_.emplace_back(0xFFffd8b1);
   colorList_.emplace_back(0xFF000075);
   colorList_.emplace_back(0xFFa9a9a9);
   colorList_.emplace_back(0xFFe6194B);
   colorList_.emplace_back(0xFFffffff);
   colorList_.emplace_back(0xFF000000);
}

DataType GraphArea::GetCentralAxis()
{
    return centralAxis_;
}

void GraphArea::SetCentralAxis(DataType n)
{
    centralAxis_ = n;

    for (auto &graph : graphics_)
        graph->centralAxis = n;
}

bool GraphArea::AddGraphics(Graphics *graph)
{
    if (!colorList_.empty()
     && graph->SetColor(colorList_[colorIdx_]))
    {
        colorIdx_++;
        if (colorIdx_ == (int) colorList_.size())
            colorIdx_ = 0;
    }
    graph->centralAxis = centralAxis_;
    graphics_.emplace_back(graph);
    return true;
}

void GraphArea::MinMaxData()
{
    if (validCache_)
        return;

    cacheMin_ = INT16_MAX;
    cacheMax_ = INT16_MIN;

    DataSet& data = panel_->DataRef();

    int end = min(end_, data.RowCount());
    if (end - begin_ == 0)
        return;

    for (const auto &item: graphics_)
    {
        int colNum = (int) item->cids.size();
        for (int i = 0; i < colNum; i++)
        {
            DataRows& col = data[item->cids[i]->index];
            for (int j = begin_; j < end; j++)
            {
                if (col[j] < cacheMin_)
                    cacheMin_ = col[j];

                if (col[j] > cacheMax_)
                    cacheMax_ = col[j];
            }
        }
    }

    if (!isnan(centralAxis_))
    {
        DataType maxDis = cacheMax_ - centralAxis_;
        DataType minDis = centralAxis_ - cacheMin_;
        if (maxDis > minDis)
            cacheMin_ = centralAxis_ - maxDis;
        else
            cacheMax_ = centralAxis_ + minDis;
    }
}

void GraphArea::UpdateScales()
{
    if (begin_ == end_)
        return;

    scales_.clear();

    if (graphics_.empty())
        return;

    // 计算刻度
    DataSet& data = panel_->DataRef();

    Scalar contentHeight = GetContentHeight();

    // int digit = 0;
    int distance = 40;

    Scalar scaleNum = contentHeight / distance;
    if (scaleNum <= 0)
        scaleNum = 1;

    float differ_gap = float(cacheMax_ - cacheMin_) / float(scaleNum);

    float exponent = log10(differ_gap) - 1;
    int exponenti = (int) exponent;
    if (exponent < 0 && abs(exponent) > 1e-8)
        exponenti--;

    int tmp_step = (int) (differ_gap / pow(10, exponenti));

    const int stdlen = 5;
    const int standard[stdlen] = {10, 20, 25, 50, 100};
    int fixed = 10;
    for (int i = stdlen - 1; i >= 1; i--)
    {
        if (tmp_step > (standard[i] + standard[i - 1]) / 2)
        {
            fixed = standard[i];
            break;
        }
    }

    float step = float(fixed * pow(10, exponenti));

    if (tmp_step > 0)
    {
        float start = 0;
        if (cacheMin_ >= 0) {
            //start = (min / step) * tmp_step + tmp_step;
            while (start < cacheMin_) {
                start += step;
            }
        } else {
//            start = (min / tmp_step) * tmp_step - tmp_step;
            while (start > cacheMin_) {
                start -= step;
            }
        }
        while (start <= cacheMax_) {
            scales_.push_back(start);
            start += step;
        }
    }

    lAxis_->OnSetScales(scales_);
    rAxis_->OnSetScales(scales_);
}

void GraphArea::OnFitIdx(int begin, int end)
{
    begin_ = begin;
    end_ = end;

    validCache_ = false;

    MinMaxData();
}

void GraphArea::OnPaint(GraphContext *ctx)
{
    // TODO: 标题

    Size size = {
        (bounds_.right - bounds_.left),
        (bounds_.bottom - bounds_.top)
    };

    if (graphics_.empty())
        return;

    DataSet& raw = panel_->DataRef();

    int wCount = end_ - begin_;
    DataType hCount = cacheMax_ - cacheMin_;
    float wratio = float(size.width) / float(wCount);
    float hratio = float(size.height) / float(hCount);

    DrawData data(raw, begin_, min(wCount, raw.RowCount()),
                  size, wratio, hratio, cacheMin_);

    // 绘制刻度线
    ctx->SetColor(scaleLineColor_);
    for (auto &item : scales_)
    {
        Scalar y = data.ToPY(item);
        ctx->DrawLine(0, y, size.width, y);
    }

    // 绘制图形
    OnPaintGraph(ctx, data);

    // 绘制刻度
    ctx->SetTranslate({bounds_.left-lAxis_->GetWidth(), bounds_.top});
    lAxis_->OnPaint(ctx, data, size.height);

    ctx->SetTranslate({bounds_.right, bounds_.top});
    rAxis_->OnPaint(ctx, data, size.height);
}

void GraphArea::OnPaintGraph(GraphContext *ctx, DrawData& data)
{
    for (const auto &item: graphics_)
        item->Paint(ctx, data);
}

}