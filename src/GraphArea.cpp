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

#include "GraphArea.h"
#include "KChartWnd.h"

#include <atlstr.h>

namespace kchart {

// 使用 sse 加速查找最大最小值
void sse_min_max(
    int64_t *p,
    int count,
    int *pmini,
    int64_t *pmin,
    int *pmaxi,
    int64_t *pmax
) {
//  __m256i ma = _mm256_set1_epi64x(*pmin);
//  __m256i mb = _mm256_set1_epi64x(*pmax);
//  __m256i mc;
//  int gCount = count / 4;
//  for (int i = 0; i < gCount; ++i) {
//    mc = _mm256_load_si256((__m256i*)p);
//    ma = _mm256_min_epu64(ma, mc);
//    mb = _mm256_max_epu64(mb, mc);
//    p += 4;
//  }
//  *pmin = min(
//      min(ma.m256i_i64[0], ma.m256i_i64[1]),
//      min(ma.m256i_i64[2], ma.m256i_i64[3])
//  );
//
//  *pmax = max(
//      max(mb.m256i_i64[0], mb.m256i_i64[1]),
//      max(mb.m256i_i64[2], mb.m256i_i64[3])
//  );
//  for (int i = 0; i < count % 4; ++i) {
//    *pmin = min(*pmin, p[i]);
//    *pmax = max(*pmax, p[i]);
//  }
  int off = 0;
  for (; off < count; off++) {
    if (*p != KC_INVALID_DATA)
      break;
    p++;
  }

  int gCount = count >> 2;
  for (int i = 0; i < gCount; ++i) {
    if (p[0] > *pmax) {*pmax = p[0]; *pmaxi = (i<<2);   }
    if (p[1] > *pmax) {*pmax = p[1]; *pmaxi = (i<<2)+1; }
    if (p[2] > *pmax) {*pmax = p[2]; *pmaxi = (i<<2)+2; }
    if (p[3] > *pmax) {*pmax = p[3]; *pmaxi = (i<<2)+3; }
    if (p[0] < *pmin) {*pmin = p[0]; *pmini = (i<<2);   }
    if (p[1] < *pmin) {*pmin = p[1]; *pmini = (i<<2)+1; }
    if (p[2] < *pmin) {*pmin = p[2]; *pmini = (i<<2)+2; }
    if (p[3] < *pmin) {*pmin = p[3]; *pmini = (i<<2)+3; }
    p += 4;
  }
  for (int i = 0; i < count % 4; ++i) {
    if (*p > *pmax) { *pmax = *p; *pmaxi = (gCount>>2)+i; }
    if (*p < *pmin) { *pmin = *p; *pmini = (gCount>>2)+i; }
    p++;
  }
  *pmaxi += off;
}

// 使用 sse 加速查找最大最小值
void sse_max(
    int64_t *p,
    int count,
    int *pmaxi,
    int64_t *pmax
) {
//  __m256i ma = _mm256_set1_epi64x(*pmax);
//  __m256i mb;
//  int gCount = count / 4;
//  for (int i = 0; i < gCount; ++i) {
//    mb = _mm256_loadu_si256((__m256i*)p);
//    ma = _mm256_max_epi64(ma, mb);
//    p += 4;
//  }
//  *pmax = max(
//      max(ma.m256i_i64[0], ma.m256i_i64[1]),
//      max(ma.m256i_i64[2], ma.m256i_i64[3])
//  );
//  for (int i = 0; i < count % 4; ++i) {
//    *pmax = max(*pmax, p[i]);
//  }

  int off = 0;
  for (; off < count; off++) {
    if (*p != KC_INVALID_DATA)
      break;
    p++;
  }

  count -= off;
  int gCount = count >> 2;
  for (int i = 0; i < gCount; ++i) {
    if (p[0] > *pmax) { *pmax = p[0]; *pmaxi = (i<<2);   }
    if (p[1] > *pmax) { *pmax = p[1]; *pmaxi = (i<<2)+1; }
    if (p[2] > *pmax) { *pmax = p[2]; *pmaxi = (i<<2)+2; }
    if (p[3] > *pmax) { *pmax = p[3]; *pmaxi = (i<<2)+3; }
    p += 4;
  }
  for (int i = 0; i < count % 4; ++i) {
    if (*p > *pmax) { *pmax = *p; *pmaxi = (gCount>>2)+i; }
    p++;
  }
  *pmaxi += off;
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
    , lAxis_(la)
    , rAxis_(ra)
    , scaleLineColor_(0xFF353535) // 0xFF303030
    , colorIdx_(0)
    , centralAxis_(KC_INVALID_DATA)
    , zeroOrigin_(false)
    , decimals_(2)
    , begin_(0)
    , end_(0) {
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


void GraphArea::SetWeight(float weight) {
  weight_ = weight;
}
float GraphArea::GetWeight() const {
  return weight_;
}
void GraphArea::SetBounds(const Rect &bounds) {
  bounds_ = bounds;
}
Rect GraphArea::GetBounds() const {
  return bounds_;
}
Scalar GraphArea::GetLabelHeight() const {
  return labelVisible_ ? labelHeight_ : 0;
}
// 代表当前界面展示的所有数据的 min max
DataType GraphArea::GetMin() const {
  return cacheMin_;
}
DataType GraphArea::GetMax() const {
  return cacheMax_;
}
const DataRows &GraphArea::GetScales() {
  return scales_;
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

void GraphArea::SetZeroOrigin(bool flag) {
  zeroOrigin_ = flag;
}

void GraphArea::SetDecimals(int decimals) {
  decimals_ = decimals;
  lAxis_->SetDecimals(decimals);
  rAxis_->SetDecimals(decimals);
}

void GraphArea::SetLabelVisible(bool flag) {
  labelVisible_ = flag;
}

void GraphArea::SetLabelBackColor(Color color) {
  labelBackColor_ = color;
}

void GraphArea::SetScaleLineColor(Color color) {
  scaleLineColor_ = color;
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
  columns_.insert(columns_.end(),
                  graph->cids.begin(),
                  graph->cids.end());

  UpdateMinMax();

  lAxis_->SetPrecision(graph->cids[0]->precision);
  rAxis_->SetPrecision(graph->cids[0]->precision);
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
  int idx = crosshairIndex_;
  if (idx == -1) {
    idx = data.Count() - 1;
  }

  int count = 0;
  for (const auto &item: graphics_) {
    Color color;
    // 带中轴的图形颜色设置方式或许会不同.
    if (item->centralAxis == KC_INVALID_DATA)
      color = item->GetColor(data, idx);
    else
      color = item->GetColorWithCA(data, idx);

    for (ColumnKey key: item->cids) {
      if (count == labels_.size())
        labels_.emplace_back();

      Label &label = labels_[count++];

      DataType val = data.Get(key, idx);
      CStringW labelVal = DataToStr(val, key->precision, decimals_);
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
  // 不能直接使用 INT64_MAX 因为 INT64_MAX = KC_INVALID_DATA
  cacheMin_ = INT64_MAX - 1;
  cacheMax_ = INT64_MIN + 1;

  DataSet &data = panel_->DataRef();

  int end = min(end_, data.RowCount());
  int count = end - begin_;
  if (count == 0)
    return;

  bool invalidCA = centralAxis_ == KC_INVALID_DATA;

  if (zeroOrigin_ && invalidCA) {
    cacheMin_ = 0;
    for (auto *cid: columns_) {
      DataRows &col = data.Get(cid);
      sse_max(col.data() + begin_, count,
              &cacheMaxIdx_, &cacheMax_);
    }
  } else {
    for (auto *cid: columns_) {
      DataRows &col = data.Get(cid);
      sse_min_max(col.data() + begin_, count,
                  &cacheMinIdx_, &cacheMin_,
                  &cacheMaxIdx_, &cacheMax_);
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
  DataType step = (cacheMax_ - cacheMin_) / DataType(scaleNum);
  double exponent = log10((double)step) - 1;
  int expi = (int) exponent;
  if (exponent < 0 && abs(exponent) > 1e-8)
    expi--;

  double s = pow(10, expi);

  DataType tmp_step = DataType(double(step) / s);
  // 100 75 50 25 10
  if (tmp_step > 88)      tmp_step = 100;
  else if (tmp_step > 63) tmp_step = 75;
  else if (tmp_step > 38) tmp_step = 50;
  else if (tmp_step > 8)  tmp_step = 25;
  else tmp_step = 10;

  step = DataType(double(tmp_step) * s);

  bool update = false;
  if (step > 0) {
    DataType start = 0;
    if (centralAxis_ != KC_INVALID_DATA) {
      start = centralAxis_;
      while ((start - step) > cacheMin_) start -= step;
    }
    if (cacheMin_ >= 0) {
      while (start < cacheMin_) start += step;
    } else {
      while ((start - step) > cacheMin_) start -= step;
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
    if ((int) scales_.size() != i) {
      scales_.resize(i);
      update = true;
    }
  }

  if (update) {
    lAxis_->OnSetScales(scales_);
    rAxis_->OnSetScales(scales_);
  }
}

void GraphArea::OnCrosshairIdxChanged(GraphContext *ctx, DrawData& data) {
  if (labelVisible_) {
    ReGatherLabel(ctx, data);
  }
}

void GraphArea::OnFitIdx(int begin, int end) {
  // int off = begin - being_;

  begin_ = begin;
  end_ = end;

  validCache_ = false;

  UpdateMinMax();
}

void GraphArea::OnPaint(GraphContext *ctx, DrawData &data) {
  if (graphics_.empty())
    return;

  OnPrePaint(ctx, data);

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
  if (centralAxis_ != KC_INVALID_DATA && boldCA_) {
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

  if (mstrCrosshairIdx_ < 0) {
    if (crosshairPoint_.x != -1)
      index = data.ToIdx(crosshairPoint_.x);
    if (crosshairIndex_ != index) {
      crosshairIndex_ = index;
      ReGatherLabel(ctx, data);
    }
  } else {
     if (mstrCrosshairIdx_ != crosshairIndex_) {
       crosshairIndex_ = mstrCrosshairIdx_;
       crosshairPoint_.x = data.ToPX(crosshairIndex_);
       ReGatherLabel(ctx, data);
     }
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