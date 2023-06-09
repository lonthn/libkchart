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

#include "Graphics.h"
#include "StrUtils.h"

#include <algorithm>

namespace kchart {

void KLineGraph::Paint(GraphContext *gctx, const DrawData &data) {
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

  int count = data.Count();
  for (int i = 0; i < count; i++) {
    Scalar x = data.ToPX(i);

    DataType open = data.Get(cids[Open], i);
    DataType high = data.Get(cids[High], i);
    DataType low = data.Get(cids[Low], i);
    DataType close = data.Get(cids[Close], i);

    if (min > low) {
      min = low;
      minIdx = i;
    }
    if (max < high) {
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
    if (width == 1 || open == close) {
      if (high != low) {
        Scalar top = data.ToPY(high);
        Scalar bottom = data.ToPY(low);
        gctx->DrawLine(x, top, x, bottom);
      }
    } else {
      DataType upStart, downStart;
      upStart = max(open, close);
      downStart = min(open, close);
      if (upStart < high) {
        Scalar top = data.ToPY(high);
        Scalar bottom = data.ToPY(upStart);
        gctx->DrawLine(x, top, x, bottom);
      }
      if (downStart > low) {
        Scalar top = data.ToPY(downStart);
        Scalar bottom = data.ToPY(low);
        gctx->DrawLine(x, top, x, bottom);
      }
    }

    // 绘制箱体
    if (width > 1) {
      Scalar left = x - (reactWidth / 2);
      if (open < close) // 阳线
      {
        gctx->FillRect(
            left, data.ToPY(close),
            left + reactWidth, data.ToPY(open)
        );
      } else if (open > close) // 阴线
      {
        gctx->FillRect(
            left, data.ToPY(open),
            left + reactWidth, data.ToPY(close)
        );
      } else // 十字线
      {
        Scalar y = data.ToPY(close);
        gctx->DrawLine(left, y, left + reactWidth, y);
      }
    }
  }

  gctx->SetColor(TextColor);

  // 最高与最低点标注
  auto fn = [=](DataType num, int i) {
    Scalar dis = 10;
    CStringW str = DataToStr(num, cids[Open]->precision, Digit);
    Size size = gctx->MeasureStr(str);

    Scalar halfHeight = size.height / 2;
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
    if (i < data.Count() / 2) {
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


void PolyLineGraph::Paint(GraphContext *gctx, const DrawData &data) {
  int i = 0;
  int count = data.Count();
  for (; i < count; i++) {
    DataType val = data.Get(cids[0], i);
    if (val != KC_INVALID_DATA)
      break;
  }

  gctx->SetColor(GetColor(data, 0));
  gctx->SetLineWidth(LineWidth);

  gctx->BeginPolyLine(count - i);
  for (; i < count; i++) {
    DataType val = data.Get(cids[0], i);
    Scalar x = data.ToPX(i);
    Scalar y = data.ToPY(val);
    gctx->AddPolyLine({x, y});
  }
  gctx->EndPolyLine();
}

void HistogramGraph::Paint(GraphContext *gctx, const DrawData &data) {
  bool validca = centralAxis != KC_INVALID_DATA;
  DataType baseLine = validca ? centralAxis : 0;
  Scalar width = FixedWidth != -1
                 ? FixedWidth : data.sWidth - data.sMargin * 2;

  if (width == data.sWidth)
    width = 1;

  // gctx->SetColor(NormalColor);

  int count = data.Count();
  for (int i = 0; i < count; i++) {
    DataType val = data.Get(cids[0], i);

    if (baseLine == val)
      continue;

    if (validca)
      gctx->SetColor(GetColorWithCA(data, i));
    else
      gctx->SetColor(GetColor(data, i));

    Scalar x = data.ToPX(i);
    Scalar top = data.ToPY(val);
    Scalar bottom = data.ToPY(baseLine);
    if (width == 1) {
      gctx->DrawLine(x, top, x, bottom);
    } else {
      Scalar left = x - width / 2;
      gctx->FillRect(
          left, top,
          left + width, bottom
      );
    }
  }
}

}
