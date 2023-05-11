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
, labelVisible_(true)
, labelHeight_(20)
, labelBackColor_(0xFF2C2F36) //0xFF1C1F26
, crosshairPoint_({-1, -1})
, lAxis_(la)
, rAxis_(ra)
, scaleLineColor_(0xFF303030)
, colorIdx_(0)
, centralAxis_(NAN)
{
   colorList_.emplace_back(0xFF4cc45b);
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

Scalar GraphArea::GetContentHeight() const
{
    Scalar height = bounds_.bottom - bounds_.top;
    if (labelVisible_)
        height -= labelHeight_;

    return height;
}

void GraphArea::MinMaxData()
{
    cacheMin_ = INT16_MAX;
    cacheMax_ = INT16_MIN;

    DataSet& data = panel_->DataRef();

    int end = min(end_, data.RowCount());
    if (end - begin_ == 0)
        return;

    for (const auto &item : graphics_)
    {
        for (auto *cid : item->cids)
        {
            DataRows& col = data[cid];
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

void GraphArea::OnMoveCrosshair(Point point)
{
    crosshairPoint_ = point;
}

void GraphArea::OnPaint(GraphContext *ctx)
{
    // TODO: 标签

    Size size = {
        (bounds_.right - bounds_.left),
        (bounds_.bottom - bounds_.top)
    };

    if (graphics_.empty())
        return;

    Scalar offY = 0;
    if (labelVisible_)
    {
        offY = labelHeight_;
        size.height -= labelHeight_;
    }

    DataSet& raw = panel_->DataRef();

    int wCount = end_ - begin_;
    DataType hCount = cacheMax_ - cacheMin_;

    DrawData data(raw, begin_, min(wCount, raw.RowCount()));
    data.size = size;
    data.bias = cacheMin_;
    data.wRatio = float(size.width) / float(wCount);
    data.hRatio = float(size.height) / float(hCount);
    data.sWidth  = Scalar(data.wRatio);
    if (data.sWidth < 3)
        data.sWidth = 1;
    data.sMargin = data.sWidth / 4;

    if (labelVisible_)
        OnPaintLabel(ctx, data);

    ctx->Translate({0, offY});

    // 绘制刻度线
    ctx->SetColor(scaleLineColor_);
    for (auto &item : scales_)
    {
        Scalar y = data.ToPY(item);
        ctx->DrawLine(0, y, size.width, y);
    }

    // 绘制图形
    OnPaintGraph(ctx, data);

    // 绘制十字准线
    OnPaintCrosshair(ctx, data);

    // 绘制刻度轴 TODO: 临时方案，这会导致坐标原点很乱 @_@
    ctx->SetTranslate({bounds_.left-lAxis_->GetWidth(), bounds_.top + offY});
    lAxis_->OnPaint(ctx, data, size.height);

    ctx->SetTranslate({bounds_.right, bounds_.top + offY});
    rAxis_->OnPaint(ctx, data, size.height);
}

void GraphArea::OnPaintLabel(GraphContext *ctx, DrawData& data)
{
    int index = data.Count() - 1;

    Scalar offX = 0;
    Scalar width = bounds_.right - bounds_.left;

    if (crosshairPoint_.x != -1)
        index = data.ToIdx(crosshairPoint_.x);

    ctx->SetColor(labelBackColor_);
    ctx->SetFont(FontId_WRYH, 10);
    ctx->FillRect(0, 0, width, labelHeight_);

    wchar_t txtBuf[128] = {0};
    for (const auto &item : graphics_)
    {
        if (isnan(item->centralAxis))
            ctx->SetColor(item->GetColor(data, index));
        else
            ctx->SetColor(item->GetColorWithCA(data, index));

        for (ColumnKey key : item->cids)
        {
            WStr labelName = WStr::FromUTF8(key->name);
            DataType val = data.Get(key, index);
            WStr labelVal = WStr::ToString(val, item->Digit);

            swprintf(txtBuf, L"%s:%s ", labelName.c_str(), labelVal.c_str());

            Size size = ctx->MeasureStr(txtBuf);
            if (offX + size.width > width)
                return;

            ctx->DrawStr(txtBuf, {offX, (labelHeight_ - size.height) / 2});
            offX += size.width;
        }
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