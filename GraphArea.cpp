//
// Created by luo-zeqi on 2013/4/18.
//

#include "GraphArea.h"
#include "KChartWnd.h"

#include <atlstr.h>

namespace kchart {

GraphArea::~GraphArea()
{
}

GraphArea::GraphArea(KChartWnd *panel,
                     VerticalAxis *la,
                     VerticalAxis *ra)
: panel_(panel)
, validCache_(false)
, labelVisible_(true)
, labelHeight_(20)
, labelBackColor_(0xFF2C2F36) //0xFF1C1F26
, crosshairPoint_({-1, -1})
, crosshairIndex_(-1)
, lAxis_(la)
, rAxis_(ra)
, scaleLineColor_(0xFF303030)
, colorIdx_(0)
, centralAxis_(NAN)
{
   colorList_.emplace_back(0xFFFFFFFF);
   colorList_.emplace_back(0xFFfff129);
   colorList_.emplace_back(0xFF5373e8);
   colorList_.emplace_back(0xFFf59241);
   colorList_.emplace_back(0xFFa12ec4);
   colorList_.emplace_back(0xFF42d4f4);
   colorList_.emplace_back(0xFFf032e6);
   colorList_.emplace_back(0xFFbfef45);
   colorList_.emplace_back(0xFFfabed4);
   colorList_.emplace_back(0xFF469990);
   colorList_.emplace_back(0xFFdcbeff);
   colorList_.emplace_back(0xFF9A6324);
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

bool GraphArea::AddGraphics(const std::vector<Graphics *>& graph) {
    for (auto *item : graph)
        AddGraphics(item);
    return true;
}

Scalar GraphArea::GetContentTop() const
{
    return labelVisible_ ? labelHeight_ : 0;
}

Scalar GraphArea::GetContentHeight() const
{
    Scalar height = bounds_.bottom - bounds_.top;
    if (labelVisible_)
        height -= labelHeight_;

    return height;
}

void GraphArea::ReGatherLabel(GraphContext *ctx, DrawData& data)
{
    int count = 0;
    for (const auto &item : graphics_)
    {
        Color color;
        if (isnan(item->centralAxis))
            color = item->GetColor(data, crosshairIndex_);
        else
            color = item->GetColorWithCA(data, crosshairIndex_);

        for (ColumnKey key : item->cids)
        {
            if (count == labels_.size())
                labels_.emplace_back();

            Label& label = labels_[count++];

            label.color = color;

            DataType val = data.Get(key, crosshairIndex_);
            CStringW labelVal = DoubleToStr(val, item->Digit);
            label.text.Format(
                L"%s:%s ",
                key->name.GetString(),
                labelVal.GetString()
            );
            label.size = ctx->MeasureStr(label.text);
        }
    }
}

void GraphArea::UpdateMinMax()
{
    cacheMin_ = INT16_MAX;
    cacheMax_ = INT16_MIN;

    DataSet& data = panel_->DataRef();

    int end = min(end_, data.RowCount());
    if (end - begin_ == 0)
        return;

    for (const auto &item : graphics_)
    {
        for (auto *cid : item->cids) {
            DataRows& col = data[cid];
            for (int j = begin_; j < end; j++) {
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

    int distance = 40;
    Scalar contentHeight = GetContentHeight();

    // 刻度数量，至少有一个
    Scalar scaleNum = contentHeight / distance;
    if (scaleNum <= 0)
        scaleNum = 1;

    // 刻度之间的间隔
    float step = float(cacheMax_ - cacheMin_) / float(scaleNum);
    float exponent = log10(step) - 1;
    int expi = (int) exponent;
    if (exponent < 0 && abs(exponent) > 1e-8)
        expi--;

    double s = pow(10, expi);

    int tmp_step = int(step / s);
    int mod = tmp_step % 10;
    if (mod > 0)
    {
        if (mod > 5)
            tmp_step += 10 - mod;
        else
            tmp_step -= mod;
    }

    step = float(tmp_step * s);

    if (step > 0)
    {
        float start = 0;
        if (cacheMin_ >= 0) {
            while (start < cacheMin_) {
                start += step;
            }
        } else {
            while ((start-step) > cacheMin_) {
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

    UpdateMinMax();
}

void GraphArea::OnMoveCrosshair(Point point)
{
    crosshairPoint_ = point;
}

void GraphArea::OnPaint(GraphContext *ctx, DrawData& data)
{
    if (graphics_.empty())
        return;

    Scalar offY = 0;
    if (labelVisible_)
    {
        offY = labelHeight_;
        OnPaintLabel(ctx, data);
    }

    ctx->Translate({0, offY});

    // 绘制刻度线
    Scalar width = bounds_.Width();
    ctx->SetColor(scaleLineColor_);
    for (auto &item : scales_)
    {
        Scalar y = data.ToPY(item);
        ctx->DrawLine(0, y, width, y);
    }

    // 绘制图形
    OnPaintGraph(ctx, data);

    // 绘制十字准线
    OnPaintCrosshair(ctx, data);

    ctx->Translate({0, -offY});
}

void GraphArea::OnPaintLabel(GraphContext *ctx, DrawData& data)
{
    int index = data.Count() - 1;

    Scalar width = bounds_.Width();

    if (crosshairPoint_.x != -1)
        index = data.ToIdx(crosshairPoint_.x);

    if (crosshairIndex_ != index)
    {
        crosshairIndex_ = index;
        ReGatherLabel(ctx, data);
    }

    ctx->SetColor(labelBackColor_);
    ctx->FillRect(0, 0, width, labelHeight_);

    ctx->SetFont(FontId_WRYH, 10);

    Scalar offX = 0;
    for (Label& label : labels_)
    {
        if (offX + label.size.width > width)
            return;

        Scalar y = (labelHeight_ - label.size.height) / 2;
        ctx->SetColor(label.color);
        ctx->DrawStr(label.text, {offX, y});
        offX += label.size.width;
    }
}

void GraphArea::OnPaintGraph(GraphContext *ctx, DrawData& data)
{
    for (const auto &item: graphics_)
        item->Paint(ctx, data);
}

void GraphArea::OnPaintCrosshair(GraphContext *ctx, DrawData& data)
{
    if (crosshairPoint_.x == -1
     && crosshairPoint_.y == -1)
        return;

    ctx->SetColor(panel_->GetCrosshairColor());

    Scalar height = GetContentHeight();
    Scalar width = (bounds_.right - bounds_.left);

    if (crosshairPoint_.x != -1)
    {
        ctx->DrawLine(crosshairPoint_.x, 0,
                      crosshairPoint_.x, height);
    }
    if (crosshairPoint_.y != -1)
    {
        ctx->DrawLine(0, crosshairPoint_.y,
                      width, crosshairPoint_.y);
    }
}

}