//
// Created by luo-zeqi on 2013/4/18.
//

#include "GraphArea.h"
#include "KChartWnd.h"

#include <atlstr.h>

namespace kchart {

// 使用 sse 加速查找最大最小值
void sse_min_max(
    float *p,
    int count,
    float *pmin,
    float *pmax
) {
  __m128 ma = _mm_set_ps1(*pmin);
  __m128 mb = _mm_set_ps1(*pmax);
  __m128 mc;
  int gCount = count / 4;
  for (int i = 0; i < gCount; ++i) {
    mc = _mm_load_ps(p);
    ma = _mm_min_ps(ma, mc);
    mb = _mm_max_ps(mb, mc);
    p += 4;
  }
  *pmin = min(
      min(ma.m128_f32[0], ma.m128_f32[1]),
      min(ma.m128_f32[2], ma.m128_f32[3])
  );
  *pmax = max(
      max(mb.m128_f32[0], mb.m128_f32[1]),
      max(mb.m128_f32[2], mb.m128_f32[3])
  );
  for (int i = 0; i < count % 4; ++i) {
    *pmin = min(*pmin, p[i]);
    *pmax = max(*pmax, p[i]);
  }
}

// 使用 sse 加速查找最大最小值
void sse_max(
    float *p,
    int count,
    float *pmax
) {
  __m128 ma = _mm_set_ps1(*pmax);
  __m128 mb;
  int gCount = count / 4;
  for (int i = 0; i < gCount; ++i) {
    mb = _mm_load_ps(p);
    ma = _mm_max_ps(ma, mb);
    p += 4;
  }
  *pmax = max(
      max(ma.m128_f32[0], ma.m128_f32[1]),
      max(ma.m128_f32[2], ma.m128_f32[3])
  );
  for (int i = 0; i < count % 4; ++i) {
    *pmax = max(*pmax, p[i]);
  }
}

GraphArea::~GraphArea() {
}

GraphArea::GraphArea(KChartWnd *panel,
                     VerticalAxis *la,
                     VerticalAxis *ra)
    : panel_(panel)
    , validCache_(false)
    , labelVisible_(true)
    , labelHeight_(20)
    , labelBackColor_(0xFF3F3F3F) //0xFF1C1F26
    , crosshairPoint_({-1, -1})
    , crosshairIndex_(-1), lAxis_(la)
    , rAxis_(ra)
    , scaleLineColor_(0xFF353535) // 0xFF303030
    , colorIdx_(0)
    , centralAxis_(NAN) {
  colorList_.emplace_back(0xFF416DF9);
  colorList_.emplace_back(0xFFfff129);
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

DataType GraphArea::GetCentralAxis() {
  return centralAxis_;
}

void GraphArea::SetCentralAxis(DataType n) {
  centralAxis_ = n;

  for (auto &graph: graphics_)
    graph->centralAxis = n;
}

void GraphArea::SetBoldCentralAxis(bool flag) {
  boldCA_ = flag;
}

void GraphArea::SetLabelVisible(bool flag) {
  labelVisible_ = flag;
}


bool GraphArea::AddGraphics(Graphics *graph) {
  if (!colorList_.empty()
      && graph->SetColor(colorList_[colorIdx_])) {
    colorIdx_++;
    if (colorIdx_ == (int) colorList_.size())
      colorIdx_ = 0;
  }
  graph->centralAxis = centralAxis_;
  graphics_.emplace_back(graph);
  UpdateMinMax();
  return true;
}

bool GraphArea::AddGraphics(const std::vector<Graphics *> &graph) {
  for (auto *item: graph)
    AddGraphics(item);
  return true;
}

Scalar GraphArea::GetContentTop() const {
  return labelVisible_ ? labelHeight_ : 0;
}

Scalar GraphArea::GetContentHeight() const {
  Scalar height = bounds_.bottom - bounds_.top;
  if (labelVisible_)
    height -= labelHeight_;

  return height;
}

void GraphArea::ReGatherLabel(GraphContext *ctx, DrawData &data) {
  int count = 0;
  for (const auto &item: graphics_) {
    Color color;
    // 带中轴的图形颜色设置方式或许会不同.
    if (isnan(item->centralAxis))
      color = item->GetColor(data, crosshairIndex_);
    else
      color = item->GetColorWithCA(data, crosshairIndex_);

    for (ColumnKey key: item->cids) {
      if (count == labels_.size())
        labels_.emplace_back();

      Label &label = labels_[count++];

      DataType val = data.Get(key, crosshairIndex_);
      CStringW labelVal = DoubleToStr(val, item->Digit);
      label.text.Format(
          L"%s:%s ",
          key->name.GetString(),
          labelVal.GetString()
      );
      label.size = ctx->MeasureStr(label.text);
      label.color = color;
    }
  }
}

void GraphArea::UpdateMinMax() {
  cacheMin_ = 1e10;
  cacheMax_ = -1e10;

  DataSet &data = panel_->DataRef();

  int end = min(end_, data.RowCount());
  int count = end - begin_;
  if (count == 0)
    return;

  bool invalidCA = isnan(centralAxis_);

  for (const auto &item: graphics_) {
    if (item->ZeroOrigin && invalidCA) {
      cacheMin_ = 0;
      for (auto *cid: item->cids) {
        DataRows &col = data.Get(cid);
        sse_max(col.data() + begin_, count,
                &cacheMax_);
      }
    } else {
      for (auto *cid: item->cids) {
        DataRows &col = data.Get(cid);
        sse_min_max(col.data() + begin_, count,
                    &cacheMin_, &cacheMax_);
      }
    }
  }

  if (!invalidCA) {
    DataType maxDis = cacheMax_ - centralAxis_;
    DataType minDis = centralAxis_ - cacheMin_;
    if (maxDis > minDis)
      cacheMin_ = centralAxis_ - maxDis;
    else
      cacheMax_ = centralAxis_ + minDis;
  }
}

void GraphArea::UpdateScales() {
  if (begin_ == end_)
    return;

  if (graphics_.empty()) {
    scales_.clear();
    return;
  }

  int distance = 40;
  Scalar contentHeight = GetContentHeight();

  // 刻度数量，至少要有一个
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
  // 100 75 50 25 10
  if (tmp_step > 88)
    tmp_step = 100;
  else if (tmp_step > 63)
    tmp_step = 75;
  else if (tmp_step > 38)
    tmp_step = 50;
  else if (tmp_step > 8)
    tmp_step = 25;
  else
    tmp_step = 10;

  step = float(tmp_step * s);

  bool update = false;
  if (step > 0) {
    float start = 0;
    if (!isnan(centralAxis_)) {
      start = centralAxis_;
      while (start > cacheMin_)
        start -= step;
    }
    if (cacheMin_ >= 0) {
      while (start < cacheMin_)
        start += step;
    } else {
      while ((start - step) > cacheMin_)
        start -= step;
    }

    int i = 0;
    while (start <= cacheMax_) {
      if (scales_.size() == i) {
        scales_.push_back(start);
        update = true;
      } else if (scales_[i] != start) {
        scales_[i] = start;
        update = true;
      }
      start += step;
      i++;
    }
    scales_.resize(i);
  }

  if (update) {
    lAxis_->OnSetScales(scales_);
    rAxis_->OnSetScales(scales_);
  }
}

void GraphArea::OnFitIdx(int begin, int end) {
  begin_ = begin;
  end_ = end;

  validCache_ = false;

  UpdateMinMax();
}

void GraphArea::OnMoveCrosshair(Point point) {
  crosshairPoint_ = point;
  lAxis_->OnMoveCrosshair(point);
  rAxis_->OnMoveCrosshair(point);
}

void GraphArea::OnPaint(GraphContext *ctx, DrawData &data) {
  if (graphics_.empty())
    return;

  Scalar offY = 0;
  if (labelVisible_) {
    offY = labelHeight_;
    OnPaintLabel(ctx, data);
  }

  ctx->Translate({0, offY});

  // 绘制刻度线
  Scalar width = bounds_.Width();
  ctx->SetLineWidth(3);
  ctx->SetColor(labelBackColor_);
  if (!isnan(centralAxis_) && boldCA_) {
    Scalar y = data.ToPY(centralAxis_);
    ctx->DrawLine(0, y, width, y);
  }
  ctx->SetLineWidth(1);
  ctx->SetColor(scaleLineColor_);
  for (auto &item: scales_) {
    Scalar y = data.ToPY(item);
    ctx->DrawLine(0, y, width, y);
  }


  // 绘制图形
  OnPaintGraph(ctx, data);

  // 绘制十字准线
  OnPaintCrosshair(ctx, data);

  ctx->Translate({0, -offY});
}

void GraphArea::OnPaintLabel(GraphContext *ctx, DrawData &data) {
  int index = data.Count() - 1;

  Scalar width = bounds_.Width();

  if (crosshairPoint_.x != -1)
    index = data.ToIdx(crosshairPoint_.x);

  if (crosshairIndex_ != index) {
    crosshairIndex_ = index;
    ReGatherLabel(ctx, data);
  }

  ctx->SetColor(labelBackColor_);
  ctx->FillRect(0, 0, width, labelHeight_);

  ctx->SetFont(FontId_WRYH, 10);

  Scalar offX = 0;
  for (Label &label: labels_) {
    if (offX + label.size.width > width)
      return;

    Scalar y = (labelHeight_ - label.size.height) / 2;
    ctx->SetColor(label.color);
    ctx->DrawStr(label.text, {offX, y});
    offX += label.size.width;
  }
}

void GraphArea::OnPaintGraph(GraphContext *ctx, DrawData &data) {
  for (const auto &item: graphics_)
    item->Paint(ctx, data);
}

void GraphArea::OnPaintCrosshair(GraphContext *ctx, DrawData &data) {
  if (crosshairPoint_.x == -1
      && crosshairPoint_.y == -1)
    return;

  ctx->SetColor(panel_->GetCrosshairColor());

  Scalar height = GetContentHeight();
  Scalar width = (bounds_.right - bounds_.left);

  if (crosshairPoint_.x != -1) {
    ctx->DrawLine(crosshairPoint_.x, 0,
                  crosshairPoint_.x, height);
  }
  if (crosshairPoint_.y != -1) {
    ctx->DrawLine(0, crosshairPoint_.y,
                  width, crosshairPoint_.y);
  }
}

}