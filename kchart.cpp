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

#include "kchart.h"

#include <algorithm>
#include <cassert>


namespace kchart {

#pragma comment(lib, "gdiplus.lib")


// 如果将数据完整展示，我们可能没有足够的空间，所以可以
// 尝试带上单位.
CStringW ToStrWithUnit(
  DataType val,
  int precision,
  int decimals
) {
  int64_t num = val / precision;
  if (num < 10000)
    return DataToStr(val, precision, decimals);

  double dval = double(val) / double(precision);
  CStringW unit;
  if (num < 100000000) {
    unit = L"万";
    dval /= 10000;
  }
  else {
    unit = L"亿";
    dval /= 100000000;
  }

#define FORMAT_OUT_WITH_UNIT(format_str) \
str.Format(L##format_str, dval, unit.GetString())

  CStringW str;
  switch (decimals) {
  case 0: str.Format(L"%lld%s", val / precision, unit.GetString()); break;
  case 1: FORMAT_OUT_WITH_UNIT("%.01f%s"); break;
  case 2: FORMAT_OUT_WITH_UNIT("%.02f%s"); break;
  case 3: FORMAT_OUT_WITH_UNIT("%.03f%s"); break;
  case 4: FORMAT_OUT_WITH_UNIT("%.04f%s"); break;
  case 5: FORMAT_OUT_WITH_UNIT("%.05f%s"); break;
  case 6: FORMAT_OUT_WITH_UNIT("%.06f%s"); break;
  case 7: FORMAT_OUT_WITH_UNIT("%.07f%s"); break;
  case 8: FORMAT_OUT_WITH_UNIT("%.08f%s"); break;
  case 9: FORMAT_OUT_WITH_UNIT("%.09f%s"); break;
  default: str.Format(L"%lld", val / precision, unit.GetString());
  }

  return str;
}

CStringW DataToStr(
  DataType val,
  int precision,
  int decimals
)
{
  wchar_t txt[64] = { 0 };
  DataType number = val / precision;
  DataType decima = val % precision;

  int len = swprintf_s(txt, 64 - 1, L"%lld", number);
  if (decimals <= 0)
    return CStringW(txt);

  txt[len++] = '.';

  int decimal_start = len;
  swprintf_s(&txt[len], 64 - len - 1, L"%lld", decima);

  for (int i = decimal_start; i < decimal_start + decimals; i++)
    if (txt[i] == '\0') txt[i] = '0';

  txt[len + decimals] = '\0';
  return CStringW(txt);
}

void StrSplit(
  const char* str,
  const char* delis,
  bool ignore_space,
  std::vector<std::string>& out
)
{
  if (!*str)
    return;

  int delislen = (int)strlen(delis);

  while (true) {
    if (ignore_space && *str == ' ') {
      str++;
      continue;
    }
    const char* s = strstr(str, delis);
    if (!s) {
      out.emplace_back(str);
      return;
    }

    if (s == str && ignore_space) {
      str += delislen;
      continue;
    }

    int len = static_cast<int>(s - str);
    out.emplace_back(str, len);
    str = s + delislen;

    if (!*str)
      return;
  }
}

/* GdiPlusGC ----------------------------------------------------------*/

ULONG_PTR GdiPlusToken = 0;

static HBITMAP CreateBitmap(HDC dc, int width, int height, void** bytes) {
  BITMAPINFO bmi = { 0 };
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = width * height * 4;

  return ::CreateDIBSection(dc, &bmi, DIB_RGB_COLORS,
    bytes, nullptr, 0);
}

GdiPlusGC::GdiPlusGC()
  : memDC_(NULL), memBitmap_(NULL), cacheSize_({ 0 }), offset_({ 0 }) {
  if (GdiPlusToken == 0) {
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&GdiPlusToken, &input, NULL);
  }

  memDC_ = CreateCompatibleDC(NULL);
  gdiGraph_ = NULL;
  gdiPen_ = new Gdiplus::Pen(Gdiplus::Color::White);
  gdiBrush_ = new Gdiplus::SolidBrush(Gdiplus::Color::White);

  SetFont(FontId_WRYH, 8);
  //SetFont(FontId_Arial, 8);
}

GdiPlusGC::~GdiPlusGC() {
  delete gdiBrush_;
  delete gdiPen_;

  if (memBitmap_ != NULL)
    DeleteObject(memBitmap_);

  DeleteDC(memDC_);
}

void GdiPlusGC::AllocBuffer(const Size& size) {
  if (memBitmap_ == NULL
    || size.width > cacheSize_.width
    || size.height > cacheSize_.height) {
    //        memBitmap_ = CreateCompatibleBitmap(
    //            memDC_,
    //            (int) size.width,
    //            (int) size.height
    //        );
    void* bytes = NULL;
    memBitmap_ = CreateBitmap(memDC_,
      (int)size.width,
      (int)size.height,
      &bytes);

    HGDIOBJ oldBitmap = SelectObject(memDC_, memBitmap_);

    if (oldBitmap != NULL)
      DeleteObject(oldBitmap);

    if (gdiGraph_ != NULL) {
      delete gdiGraph_;
    }

    gdiGraph_ = new Gdiplus::Graphics(memDC_);

    // 获取当前的 DPI 设置
    int dpiX, dpiY;
    dpiX = gdiGraph_->GetDpiX();
    dpiY = gdiGraph_->GetDpiY();

    // 设置缩放因子
    gdiGraph_->ScaleTransform(dpiX / 96.0f, dpiY / 96.0f);
  }

  cacheSize_ = size;
}

void GdiPlusGC::SwapBuffer(void* native) {
  int cx = (int)cacheSize_.width;
  int cy = (int)cacheSize_.height;
  ::BitBlt((HDC)native, 0, 0, cx, cy, memDC_, 0, 0, SRCCOPY);
}

Point GdiPlusGC::Translate(const Point& point) {
  Point oldPoint = offset_;
  offset_.x += point.x;
  offset_.y += point.y;
  return oldPoint;
}

void GdiPlusGC::SetTranslate(const Point& point) {
  offset_ = point;
}

void GdiPlusGC::SetFont(FontId fontId, int size) {
  uint64_t key = (uint64_t(fontId) << 32) | uint64_t(size);
  Gdiplus::Font*& font = fontCache_[key];
  if (font == NULL)
    font = new Gdiplus::Font(
      fontFamilies_[fontId].GetString(),
      Gdiplus::REAL(size)
    );

  gdiFont_ = font;
}

void GdiPlusGC::SetColor(Color color) {
  color_ = color;
  gdiPen_->SetColor(color);
  gdiBrush_->SetColor(color);
}

void GdiPlusGC::SetLineWidth(int width) {
  gdiPen_->SetWidth((Gdiplus::REAL)width);
}

void GdiPlusGC::DrawRect(const Rect& r) {
  Scalar width = r.right - r.left;
  Scalar height = r.bottom - r.top;
  gdiGraph_->DrawRectangle(gdiPen_,
    offset_.x + r.left,
    offset_.y + r.top,
    width, height);
}

void GdiPlusGC::DrawRect(
  Scalar l, Scalar t, Scalar r, Scalar b
) {
  Scalar width = r - l - 1;
  Scalar height = b - t - 1;
  gdiGraph_->DrawRectangle(gdiPen_,
    offset_.x + l,
    offset_.y + t,
    width, height);
}

void GdiPlusGC::FillRect(const Rect& r) {
  Scalar width = r.right - r.left;
  Scalar height = r.bottom - r.top;
  gdiGraph_->FillRectangle(gdiBrush_,
    offset_.x + r.left,
    offset_.y + r.top,
    width, height);
}

void GdiPlusGC::FillRect(
  Scalar l, Scalar t, Scalar r, Scalar b
) {
  Scalar width = r - l;
  Scalar height = b - t;
  gdiGraph_->FillRectangle(gdiBrush_,
    offset_.x + l,
    offset_.y + t,
    width, height);
}

void GdiPlusGC::DrawLine(const Point& a, const Point& b) {
  gdiGraph_->DrawLine(gdiPen_,
    a.x + offset_.x, a.y + offset_.y,
    b.x + offset_.x, b.y + offset_.y);
}

void GdiPlusGC::DrawLine(Scalar ax, Scalar ay, Scalar bx, Scalar by) {
  gdiGraph_->DrawLine(gdiPen_,
    ax + offset_.x, ay + offset_.y,
    bx + offset_.x, by + offset_.y);
}


void GdiPlusGC::BeginPolyLine(int pointNum) {
  if ((int)pointCache_.size() < pointNum) {
    pointCache_.resize(pointNum);
  }
  pointNum_ = 0;
}

void GdiPlusGC::AddPolyLine(const Point& p) {
  pointCache_[pointNum_++] = {
      offset_.x + p.x, offset_.y + p.y
  };
}

void GdiPlusGC::EndPolyLine() {
  if (pointNum_ == 0)
    return;

  auto smode = gdiGraph_->GetSmoothingMode();
  gdiGraph_->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  gdiGraph_->DrawLines(gdiPen_,
    pointCache_.data(),
    pointNum_);
  gdiGraph_->SetSmoothingMode(smode);
  pointNum_ = 0;
}


void GdiPlusGC::DrawStr(const CStringW& str, const Point& p) {
  //    Gdiplus::GraphicsPath graphicsPathObj;
  //    Gdiplus::FontFamily fontfamily;
  //    font.GetFamily(&fontfamily);
  //    graphicsPathObj.AddString(szText,-1,&fontfamily,font.GetStyle(),font.GetSize(),\
  //                              PointF(0,0),&strFormat);

  auto smode = gdiGraph_->GetSmoothingMode();
  gdiGraph_->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  Gdiplus::PointF location(Gdiplus::REAL(p.x + offset_.x),
    Gdiplus::REAL(p.y + offset_.y));
  gdiGraph_->DrawString(str.GetString(), str.GetLength(),
    gdiFont_, location, gdiBrush_);
  gdiGraph_->SetSmoothingMode(smode);
}

Size GdiPlusGC::MeasureStr(const CStringW& str) {
  Gdiplus::StringFormat format(Gdiplus::StringAlignmentNear);

  // 接收字体的显示区域，如宽高
  Gdiplus::RectF stringRect;
  Gdiplus::RectF layoutRect(0, 0, 600, 100);
  // gdiGraph_->SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit );

  //获得字体高度与宽度stringRect
  gdiGraph_->MeasureString(str.GetString(), str.GetLength(),
    gdiFont_, layoutRect,
    &format, &stringRect);
  return { Scalar(stringRect.Width), Scalar(stringRect.Height) };
}

/* DataSet ----------------------------------------------------------------------*/
DataSet::DataSet()
  : rowCount(0)
  , oldRowCount(0) {
}

ColumnKey DataSet::CreateCol(const std::string& name) {
  if (name.empty())
    return nullptr;

  auto iter = columns.find(name);
  if (iter != columns.end())
    return &iter->second;

  USES_CONVERSION;
  CStringW wname = A2W(name.c_str());
  columns.emplace(name, ColumnInfo{ wname, 1, ColCount() });
  raw.emplace_back(rowCount);

  return &columns[name];
}

ColumnKey DataSet::CreateCol(const std::string& name, int precision) {
  ColumnKey col = CreateCol(name);
  if (col) col->precision = precision;
  return col;
}

ColumnKey DataSet::CreateCol(const std::string& name, Setter* setter) {
  ColumnKey col = CreateCol(name);
  if (col) col->setter = setter;
  return col;
}

ColumnKey DataSet::CreateCol(const std::string& name, int precision, Setter* setter) {
  ColumnKey col = CreateCol(name);
  if (col) {
    col->precision = precision;
    col->setter = setter;
  }
  return col;
}

int DataSet::AddRow() {
  for (auto& col : raw) {
    col.resize(col.size() + 1);
  }

  return rowCount++;
}

void DataSet::AddRow(int n) {
  for (auto& col : raw) {
    col.resize(col.size() + n);
  }

  rowCount += n;
}

ColumnKey DataSet::FindCol(const std::string& name) {
  auto iter = columns.find(name);
  if (iter != columns.end())
    return &iter->second;

  return nullptr;
}

DataType DataSet::Get(int col, int row) {
  assert(col < (int)raw.size());
  assert(row < rowCount);
  return raw[col][row];
}

DataType DataSet::Get(const Index2& idx) {
  assert(idx.col < (int)raw.size());
  assert(idx.row < rowCount);
  return raw[idx.col][idx.row];
}

DataType DataSet::Get(ColumnKey col, int row) {
  assert(col->index < (int)raw.size());
  assert(row < rowCount);
  return raw[col->index][row];
}

void DataSet::Set(int col, int row, DataType val) {
  assert(col < (int)raw.size());
  assert(row < rowCount);
  raw[col][row] = val;

  //  for (auto &item: dataObservers[col])
  //    item.second({col, row});
}

void DataSet::Set(const Index2& idx, DataType val) {
  assert(idx.col < (int)raw.size());
  assert(idx.row < rowCount);
  raw[idx.col][idx.row] = val;

  //  for (auto &item: dataObservers[idx.col])
  //    item.second(idx);
}

void DataSet::Set(ColumnKey col, int row, DataType val) {
  assert(col->index < (int)raw.size());
  assert(row < rowCount);
  raw[col->index][row] = val;

  //  for (auto &item: dataObservers[col->index])
  //    item.second({col->index, row});
}

DataRows& DataSet::Get(ColumnKey col) {
  assert(col->index < (int)raw.size());
  return raw[col->index];
}

DataRows& DataSet::operator[](int col) {
  assert(col < (int)raw.size());
  return raw[col];
}


int DataSet::AddObserver(int priority, ObserverFn&& fn) {
  int id = (observerCounter++) + priority;
  observers.emplace(id, fn);
  return id;
}
void DataSet::RemoveObserver(int id) {
  observers.erase(id);
}

void DataSet::Notify() {
  for (auto& item : observers)
    item.second(*this, oldRowCount);
  oldRowCount = rowCount;
}

/* CrosshairDelegate --------------------------------------------------------*/
CrosshairDelegate::CrosshairDelegate()
  : crosshairIndex_(-1)
  , mstrCrosshairIdx_(-1)
  , forceChange_(false) {
  crosshairPoint_ = { -1, -1 };
}

Point CrosshairDelegate::GetCrosshairPoint() const {
  return crosshairPoint_;
}

int CrosshairDelegate::GetCrosshairIndex() const {
  return crosshairIndex_;
}

void CrosshairDelegate::SetForceChange() {
  forceChange_ = true;
}

void CrosshairDelegate::OnMoveCrosshair(Point point) {
  mstrCrosshairIdx_ = -1;
  crosshairPoint_ = point;
}

void CrosshairDelegate::OnMoveCrosshair(int idx) {
  mstrCrosshairIdx_ = idx;
}

void CrosshairDelegate::OnPrePaint(GraphContext* ctx, DrawData& data) {
  int index = -1;
  if (mstrCrosshairIdx_ < 0) {
    if (crosshairPoint_.x != -1)
      index = data.ToIdx(crosshairPoint_.x);
    if (forceChange_ || crosshairIndex_ != index) {
      crosshairIndex_ = index;
      OnCrosshairIdxChanged(ctx, data);
    }
  }
  else {
    if (forceChange_ || mstrCrosshairIdx_ != crosshairIndex_) {
      crosshairIndex_ = mstrCrosshairIdx_;
      crosshairPoint_.x = data.ToPX(crosshairIndex_);
      OnCrosshairIdxChanged(ctx, data);
    }
  }

  forceChange_ = false;
}

/* VerticalAxis -----------------------------------------------------------*/
void VerticalAxis::SetScaleColor(Color color) {
  scaleColor_ = color;
}

void VerticalAxis::SetCrosshairBackColor(Color color) {
  crosshairBackColor_ = color;
}

void VerticalAxis::OnSetScales(const DataRows& scales) {
  scales_.resize(scales.size());
  strScales_.resize(scales.size());

  for (int i = 0; i < (int)scales.size(); i++) {
    scales_[i] = scales[i];
    strScales_[i] = transformFn_(scales[i], precision_, decimals_);
  }
}

void VerticalAxis::OnMoveCrosshair(Point point) {
  crosshairY_ = point.y;
}

Scalar VerticalAxis::OnMeasureWidth(GraphContext* ctx) {
  Scalar width = 0;
  scaleSize_.resize(strScales_.size());
  for (int i = 0; i < strScales_.size(); i++) {
    Size size = ctx->MeasureStr(strScales_[i]);
    scaleSize_[i] = size;
    width = max(width, size.width);
  }
  return width;
}

void VerticalAxis::OnPaint(
  GraphContext* ctx,
  DrawData& data,
  Scalar offY
) {
  ctx->SetFont(FontId_WRYH, 10);

  Scalar pad = 5;

  ctx->Translate({ 0, offY });

  auto draw = [&](Scalar y, CStringW& str, bool backFill) {
    Size size = ctx->MeasureStr(str);
    Scalar left = pad;
    Scalar top = y - (size.height / 2);
    if (alignToRight_) {
      left = width_ - size.width - pad;
    }

    if (backFill) {
      ctx->SetColor(crosshairBackColor_);
      ctx->FillRect(left, top,
        left + size.width,
        top + size.height);
    }

    ctx->SetColor(scaleColor_);
    ctx->DrawStr(str, { left, top });
  };

  for (int i = 0; i < (int)scales_.size(); i++) {
    Scalar y = data.ToPY(scales_[i]);
    draw(y, strScales_[i], false);
  }

  if (crosshairY_ != -1) {
    DataType val = data.ToData(crosshairY_);
    crosshairText_ = transformFn_(val, precision_, decimals_);

    draw(crosshairY_, crosshairText_, true);
  }

  ctx->Translate({ 0, -offY });
}

/* HorizontalAxis ---------------------------------------------------------*/
HorizontalAxis::HorizontalAxis(KChartWnd* wnd)
  : master_(wnd)
  , height_(25)
  , scaleColor_(0xFF8F8F8F)
  , hdKey_(nullptr)
  , crosshairX_(-1)
  , crosshairBackColor_(0xFF3F3F3F) {
}

void HorizontalAxis::SetScaleColor(Color color) {
  scaleColor_ = color;
}

void HorizontalAxis::SetCrosshairBackColor(Color color) {
  crosshairBackColor_ = color;
}

void HorizontalAxis::OnCrosshairIdxChanged(GraphContext* ctx, DrawData& data) {
  if (crosshairIndex_ != -1) {
    DataType val = crosshairIndex_;
    if (hdKey_)
      val = data.Get(hdKey_, (int)val);
    crosshairText_.Format(L"%lld", val);
  }
}

void HorizontalAxis::OnFitIdx(
  int begin, int end
) {
  DataSet& data = master_->DataRef();
  end = min(data.RowCount(), end);
  //end

  if (end - begin == 0)
    return;

  scales_.clear();
  strScales_.clear();

  scales_.push_back(begin);
  scales_.push_back(end - 1);

  // 用户如果不提供刻度值数据列，就直接将索引展示.
  CStringW a, b;
  if (hdKey_ == nullptr) {
    a.Format(L"%d", begin);
    b.Format(L"%d", end - 1);
  }
  else {
    a.Format(L"%lld", data.Get(hdKey_, begin));
    b.Format(L"%lld", data.Get(hdKey_, end - 1));
  }
  strScales_.push_back(a);
  strScales_.push_back(b);
}

void HorizontalAxis::OnPaint(
  GraphContext* ctx,
  DrawData& data,
  Scalar width
) {
  OnPrePaint(ctx, data);

  ctx->SetFont(FontId_WRYH, 10);

  Scalar pad = 0;
  auto draw = [&](Scalar x, CStringW& str, bool backFill) {
    Size size = ctx->MeasureStr(str);
    Scalar left = x - (size.width / 2);
    Scalar top = 3;
    if (left < 0)
      left = 0;
    if ((left + size.width) > width)
      left = width - size.width;

    if (backFill) {
      ctx->SetColor(crosshairBackColor_);
      ctx->FillRect(left, top,
        left + size.width,
        top + size.height);
    }

    ctx->SetColor(scaleColor_);
    ctx->DrawStr(str, { left, top });
  };

  for (int i = 0; i < (int)scales_.size(); i++) {
    Scalar x = data.ToPX(data.NativeIdx(scales_[i]));
    draw(x, strScales_[i], false);
  }

  if (crosshairIndex_ != -1) {
    draw(crosshairPoint_.x, crosshairText_, true);
  }
}

/* Graphics ------------------------------------------------------------*/
void KLineGraph::Paint(GraphContext* gctx, const DrawData& data) {
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
    }
    else {
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
      }
      else if (open > close) // 阴线
      {
        gctx->FillRect(
          left, data.ToPY(open),
          left + reactWidth, data.ToPY(close)
        );
      }
      else // 十字线
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
    }
    else {
      gctx->DrawLine(x, ny, x - dis, ty + halfHeight);
      x -= dis + size.width;
    }

    gctx->DrawStr(str, { x, ty });
  };

  fn(min, minIdx);
  fn(max, maxIdx);
}


void PolyLineGraph::Paint(GraphContext* gctx, const DrawData& data) {
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
    gctx->AddPolyLine({ x, y });
  }
  gctx->EndPolyLine();
}

void HistogramGraph::Paint(GraphContext* gctx, const DrawData& data) {
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
    }
    else {
      Scalar left = x - width / 2;
      gctx->FillRect(
        left, top,
        left + width, bottom
      );
    }
  }
}

/* GraphArea -----------------------------------------------------------*/
// 使用 sse 加速查找最大最小值
void sse_min_max(
  int64_t* p,
  int count,
  int* pmini,
  int64_t* pmin,
  int* pmaxi,
  int64_t* pmax
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

  count -= off;
  int gCount = count >> 2;
  for (int i = 0; i < gCount; ++i) {
    if (p[0] > *pmax) { *pmax = p[0]; *pmaxi = (i << 2); }
    if (p[1] > *pmax) { *pmax = p[1]; *pmaxi = (i << 2) + 1; }
    if (p[2] > *pmax) { *pmax = p[2]; *pmaxi = (i << 2) + 2; }
    if (p[3] > *pmax) { *pmax = p[3]; *pmaxi = (i << 2) + 3; }
    if (p[0] < *pmin) { *pmin = p[0]; *pmini = (i << 2); }
    if (p[1] < *pmin) { *pmin = p[1]; *pmini = (i << 2) + 1; }
    if (p[2] < *pmin) { *pmin = p[2]; *pmini = (i << 2) + 2; }
    if (p[3] < *pmin) { *pmin = p[3]; *pmini = (i << 2) + 3; }
    p += 4;
  }
  for (int i = 0; i < count % 4; ++i) {
    if (*p > *pmax) { *pmax = *p; *pmaxi = (gCount >> 2) + i; }
    if (*p < *pmin) { *pmin = *p; *pmini = (gCount >> 2) + i; }
    p++;
  }
  *pmaxi += off;
}

// 使用 sse 加速查找最大最小值
void sse_max(
  int64_t* p,
  int count,
  int* pmaxi,
  int64_t* pmax
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
    if (p[0] > *pmax) { *pmax = p[0]; *pmaxi = (i << 2); }
    if (p[1] > *pmax) { *pmax = p[1]; *pmaxi = (i << 2) + 1; }
    if (p[2] > *pmax) { *pmax = p[2]; *pmaxi = (i << 2) + 2; }
    if (p[3] > *pmax) { *pmax = p[3]; *pmaxi = (i << 2) + 3; }
    p += 4;
  }
  for (int i = 0; i < count % 4; ++i) {
    if (*p > *pmax) { *pmax = *p; *pmaxi = (gCount >> 2) + i; }
    p++;
  }
  *pmaxi += off;
}

GraphArea::~GraphArea() {
}

GraphArea::GraphArea(KChartWnd* panel,
  VerticalAxis* la,
  VerticalAxis* ra)
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
void GraphArea::SetBounds(const Rect& bounds) {
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
const DataRows& GraphArea::GetScales() {
  return scales_;
}

DataType GraphArea::GetCentralAxis() {
  return centralAxis_;
}

void GraphArea::SetCentralAxis(DataType n) {
  centralAxis_ = n;

  for (auto& graph : graphics_)
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

bool GraphArea::AddGraphics(Graphics* graph) {
  if (!colorList_.empty()
    && graph->SetColor(colorList_[colorIdx_])) {
    colorIdx_++;
    if (colorIdx_ == (int)colorList_.size())
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

bool GraphArea::AddGraphics(const std::vector<Graphics*>& graph) {
  for (auto* item : graph)
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

void GraphArea::ReGatherLabel(GraphContext* ctx, DrawData& data) {
  int idx = crosshairIndex_;
  if (idx == -1) {
    idx = data.Count() - 1;
  }

  int count = 0;
  for (const auto& item : graphics_) {
    Color color;
    // 带中轴的图形颜色设置方式或许会不同.
    if (item->centralAxis == KC_INVALID_DATA)
      color = item->GetColor(data, idx);
    else
      color = item->GetColorWithCA(data, idx);

    for (ColumnKey key : item->cids) {
      if (count == labels_.size())
        labels_.emplace_back();

      Label& label = labels_[count++];

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

  DataSet& data = panel_->DataRef();

  int end = min(end_, data.RowCount());
  int count = end - begin_;
  if (count == 0)
    return;

  bool invalidCA = centralAxis_ == KC_INVALID_DATA;

  if (zeroOrigin_ && invalidCA) {
    cacheMin_ = 0;
    for (auto* cid : columns_) {
      DataRows& col = data.Get(cid);
      sse_max(col.data() + begin_, count,
        &cacheMaxIdx_, &cacheMax_);
    }
  }
  else {
    for (auto* cid : columns_) {
      DataRows& col = data.Get(cid);
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
  if (begin_ == end_ || panel_->DataRef().Empty())
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
  int expi = (int)exponent;
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
    }
    else {
      while ((start - step) > cacheMin_) start -= step;
    }

    int i = 0;
    while (start <= cacheMax_) {
      if (scales_.size() == i) {
        scales_.push_back(start);
        update = true;
      }
      else if (scales_[i] != start) {
        scales_[i] = start;
        update = true;
      }
      start += step;
      i++;
    }
    if ((int)scales_.size() != i) {
      scales_.resize(i);
      update = true;
    }
  }

  if (update) {
    lAxis_->OnSetScales(scales_);
    rAxis_->OnSetScales(scales_);
  }
}

void GraphArea::OnCrosshairIdxChanged(GraphContext* ctx, DrawData& data) {
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

void GraphArea::OnPaint(GraphContext* ctx, DrawData& data) {
  if (graphics_.empty() || data.Empty())
    return;

  OnPrePaint(ctx, data);

  Scalar offY = 0;
  if (labelVisible_) {
    offY = labelHeight_;
    OnPaintLabel(ctx, data);
  }

  ctx->Translate({ 0, offY });

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
  for (auto& item : scales_) {
    Scalar y = data.ToPY(item);
    ctx->DrawLine(0, y, width, y);
  }


  // 绘制图形
  OnPaintGraph(ctx, data);

  // 绘制十字准线
  OnPaintCrosshair(ctx, data);

  ctx->Translate({ 0, -offY });
}

void GraphArea::OnPaintLabel(GraphContext* ctx, DrawData& data) {
  int index = data.Count() - 1;

  Scalar width = bounds_.Width();

  if (mstrCrosshairIdx_ < 0) {
    if (crosshairPoint_.x != -1)
      index = data.ToIdx(crosshairPoint_.x);
    if (crosshairIndex_ != index) {
      crosshairIndex_ = index;
      ReGatherLabel(ctx, data);
    }
  }
  else {
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
  for (Label& label : labels_) {
    if (offX + label.size.width > width)
      return;

    Scalar y = (labelHeight_ - label.size.height) / 2;
    ctx->SetColor(label.color);
    ctx->DrawStr(label.text, { offX, y });
    offX += label.size.width;
  }
}

void GraphArea::OnPaintGraph(GraphContext* ctx, DrawData& data) {
  for (const auto& item : graphics_)
    item->Paint(ctx, data);
}

void GraphArea::OnPaintCrosshair(GraphContext* ctx, DrawData& data) {
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

/*===========================================================================================================*/
/*
 *    thunk with DEP support
 *
 *    author : proguru
 *    July 9,2008
 *
 *    link : http://www.cppblog.com/proguru/archive/2008/08/24/59831.html
 */
#ifndef __KTHUNK_H__
#define __KTHUNK_H__

#ifdef WIN32

#include <cassert>
#include <cstdint>
#include <windows.h>

 //#define USE_THISCALL_CONVENTION    //turn it off for c++ builder compatibility

#ifdef USE_THISCALL_CONVENTION
#define WNDPROC_THUNK_LENGTH    29     // For __thiscall calling convention ONLY, assign hWnd by thunk
#define GENERAL_THUNK_LENGTH    10
#define KCALLBACK                      //__thiscall is default
#else
#define WNDPROC_THUNK_LENGTH    23     //__stdcall calling convention ONLY,assign m_hWnd by thunk
#define GENERAL_THUNK_LENGTH    16
#define KCALLBACK __stdcall
#endif

static HANDLE GV_HeapExecutable = NULL;

class ThunkBase {
public:
  ThunkBase(uint32_t size) {
    if (!GV_HeapExecutable) {
      // First thunk, Create the executable heap
      GV_HeapExecutable = ::HeapCreate(
        HEAP_CREATE_ENABLE_EXECUTE, 0, 0
      );
      assert(GV_HeapExecutable);
    }

    machineCodes_ = (uint8_t*)::HeapAlloc(
      GV_HeapExecutable, HEAP_ZERO_MEMORY, size
    );
  }

  ~ThunkBase() {
    if (GV_HeapExecutable) {
      ::HeapFree(GV_HeapExecutable, 0, (void*)machineCodes_);
    }
  }

  inline LONG_PTR GetThunkedCodePtr() {
    return reinterpret_cast<LONG_PTR>(&machineCodes_[0]);
  }

protected:
  uint8_t* machineCodes_;
}; // class ThunkBase


class WndProcThunk : public ThunkBase {
public:
  WndProcThunk()
    : ThunkBase(WNDPROC_THUNK_LENGTH) {}

  void init(uintptr_t thisPtr, uintptr_t procPtr) {
#ifndef _WIN64
    uint32_t distance = (uint32_t)(procPtr)
      -(uint32_t)(&machineCodes_[0])
      - WNDPROC_THUNK_LENGTH;

#ifdef USE_THISCALL_CONVENTION
    /*
    For __thiscall, the default calling convention used by Microsoft VC++, The thing needed is
    just set ECX with the value of 'this pointer'

    machine code                       assembly instruction        comment
    ---------------------------       -------------------------    ----------
    B9 ?? ?? ?? ??                    mov ecx, pThis                ; Load ecx with this pointer
    50                                PUSH EAX
    8B 44 24 08                       MOV EAX, DWORD PTR[ESP+8]     ; EAX=hWnd
    89 01                             MOV DWORD PTR [ECX], EAX      ; [pThis]=[ECX]=hWnd
    8B 44 24 04                       mov eax,DWORD PTR [ESP+04H]   ; eax=(return address)
    89 44 24 08                       mov DWORD PTR [ESP+08h],eax   ; hWnd=(return address)
    58                                POP EAX
    83 C4 04                          add ESP,04h

    E9 ?? ?? ?? ??                    jmp ProcPtr                   ; Jump to target message handler
    */
    machineCode_[0] = 0xB9;
    *((uint32_t*)&machineCode_[1]) = (uint32_t)thisPtr;
    *((uint32_t*)&machineCode_[5]) = 0x24448B50;
    *((uint32_t*)&machineCode_[9]) = 0x8B018908;
    *((uint32_t*)&machineCode_[13]) = 0x89042444;
    *((uint32_t*)&machineCode_[17]) = 0x58082444;
    *((uint32_t*)&machineCode_[21]) = 0xE904C483;
    *((uint32_t*)&machineCode_[25]) = distance;
#else    
    /*
     * 01/26/2008 modify
    For __stdcall calling convention, replace 'HWND' with 'this pointer'

    Stack frame before modify             Stack frame after modify

    :            :                        :             :
    |---------------|                        |----------------|
    |     lParam    |                        |     lParam     |
    |---------------|                        |----------------|
    |     wParam    |                        |     wParam     |
    |---------------|                        |----------------|
    |     uMsg      |                        |     uMsg       |
    |---------------|                        |----------------|
    |     hWnd      |                        | <this pointer> |
    |---------------|                        |----------------|
    | (Return Addr) | <- ESP                 | (Return Addr)  | <-ESP
    |---------------|                        |----------------|
    :            :                        :             |

    machine code        assembly instruction            comment
    ------------------- ----------------------------    --------------
    51                  push ecx
    B8 ?? ?? ?? ??      mov  eax, pThis                  ; eax=this;
    8B 4C 24 08         mov  ecx, dword ptr [esp+08H]    ; ecx=hWnd;
    89 08               mov  dword ptr [eax], ecx        ; [this]=hWnd,if has vftbl should [this+4]=hWnd
    89 44 24 08         mov  dword ptr [esp+08H], eax    ; Overwite the 'hWnd' with 'this pointer'
    59                  pop  ecx
    E9 ?? ?? ?? ??      jmp  ProcPtr                     ; Jump to target message handler
    */

    * ((uint32_t*)&machineCodes_[0]) = 0xB851;
    *((uint32_t*)&machineCodes_[2]) = (uint32_t)thisPtr;
    *((uint32_t*)&machineCodes_[6]) = 0x08244C8B;
    *((uint32_t*)&machineCodes_[10]) = 0x4489;//0889;
    *((uint32_t*)&machineCodes_[12]) = 0xE9590824;
    *((uint32_t*)&machineCodes_[16]) = (uint32_t)distance;
#endif //USE_THISCALL_CONVENTION
#else    //_WIN64
    /*
    For x64 calling convention, RCX hold the 'HWND',copy the 'HWND' to Window object,
    then insert 'this pointer' into RCX,so perfectly!!!

    Stack frame before modify                Stack frame after modify

    :               :                        :                :
    |---------------|                        |----------------|
    |               | <-R9(lParam)           |                | <-R9(lParam)
    |---------------|                        |----------------|
    |               | <-R8(wParam)           |                | <-R8(wParam)
    |---------------|                        |----------------|
    |               | <-RDX(msg)             |                | <-RDX(msg)
    |---------------|                        |----------------|
    |               | <-RCX(hWnd)            |                | <-RCX(this)
    |---------------|                        |----------------|
    | (Return Addr) | <-RSP                  | (Return Addr)  | <-RSP
    |---------------|                        |----------------|
    :               :                        :                :

    machine code            assembly instruction     comment
    -------------------       -----------------------    ----
    48B8 ????????????????   mov RAX,pThis
    ; 4808                    mov qword ptr [RAX],RCX    ;m_hWnd=[this]=RCX
    4889C1                  mov RCX,RAX                ;RCX=pThis
    48B8 ????????????????   mov RAX,ProcPtr
    FFE0                    jmp RAX
    */
    * ((uint16_t*)&machineCodes_[0]) = 0xB848;
    *((uint64_t*)&machineCodes_[2]) = thisPtr;
    // [@author:luo-zeqi]
    // 由于使用该类的 KChartWnd 存在虚函数，其首地址所存的值被虚函数表
    // 占用，因此下面这行汇编会破坏内存，且这目前没有意义，先将其注释。
    //*((uint32_t*)&machineCodes_[10]) = 0x48088948;
    //*((uint32_t*)&machineCodes_[14]) = 0xB848C189;
    *((uint32_t*)&machineCodes_[10]) = 0x48C18948;
    *((uint32_t*)&machineCodes_[14]) = 0xB8;
    *((uint64_t*)&machineCodes_[15]) = procPtr;
    *((uint16_t*)&machineCodes_[23]) = 0xE0FF;
#endif
  }
}; // class WndProcThunk

#endif // WIN32

#endif

/*===========================================================================================================*/



bool GVClassReg = true;

KChartWnd::KChartWnd(std::shared_ptr<DataSet> data)
  : handle_(NULL)
  , procThunk_(NULL)
  , data_(data)
  , beginIdx_(0)
  , endIdx_(0)
  , size_({ 0 })
  , sWidth_(5)
  , fixedCount_(false)
  , crosshairEnable_(true)
  , crosshairVisible_(false)
  , crosshairColor_(0xFFFFFFFF)
  , crosshairPoint_({ -1, -1 })
  , lvVisible_(true)
  , rvVisible_(true)
  , vAxisWidth_(70)
  , borderColor_(0xFF3F3F3F)
  , backColor_(0xFF2B2B2B) {
  union {
    LRESULT(KCALLBACK KChartWnd::* proc)(UINT, WPARAM, LPARAM);

    uintptr_t procPtr;
  } u{};

  u.proc = &KChartWnd::OnMessage;

  procThunk_ = new WndProcThunk();
  procThunk_->init((uintptr_t)this, u.procPtr);

  gcContext_ = new GdiPlusGC();

  hAxis_ = new HorizontalAxis(this);

  data_->AddObserver(100000, [&](DataSet& data, int oldRow) {
    if (!fixedCount_) {
      int newRow = data.RowCount();
      if (oldRow != newRow) {
        if (endIdx_ == oldRow) {
          endIdx_ += newRow - oldRow;
          beginIdx_ += newRow - oldRow;
        }
      }
    }
    FitNewWidth();
    for (auto area : areas_)
      area->UpdateScales();
  });
}

KChartWnd::~KChartWnd() {
  if (gcContext_ != NULL) {
    delete gcContext_;
    gcContext_ = NULL;
  }
  if (procThunk_ != NULL) {
    delete procThunk_;
    procThunk_ = NULL;
  }

  for (int i = 0; i < areas_.size(); i++) {
    delete areas_[i];
    delete lvAxis_[i];
    delete rvAxis_[i];
  }

  delete hAxis_;

  areas_.clear();
  lvAxis_.clear();
  rvAxis_.clear();
}

bool KChartWnd::CreateWin(HWND hParent) {
  WNDCLASSA cls;

  std::string className = ClassName();
  std::string wndName = "KChartWnd";
  HINSTANCE hInstance = GetModuleHandleA(NULL);

  if (GVClassReg) {
    cls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    cls.hInstance = hInstance;
    cls.lpszClassName = (char*)className.c_str();
    cls.lpszMenuName = NULL;
    cls.lpfnWndProc = DefWindowProcA;
    cls.cbClsExtra = 0;
    cls.cbWndExtra = 0;
    cls.hIcon = NULL;
    cls.hCursor = NULL;
    cls.hbrBackground = NULL; // CreateSolidBrush(0xFFFFFF);

    ATOM atom = RegisterClassA(&cls);
    assert(atom);

    GVClassReg = false;
  }

  HWND hWnd = CreateWindowA(
    className.c_str(), wndName.c_str(),
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
    hParent, NULL, hInstance, NULL
  );

  if (hWnd == NULL)
    return false;

  handle_ = hWnd;

  LONG_PTR ptr = procThunk_->GetThunkedCodePtr();
  SetWindowLongPtrA(handle_, GWLP_WNDPROC, ptr);

  FitNewWidth();
  return true;
}

Scalar KChartWnd::Width() const {
  return size_.width;
}

Scalar KChartWnd::Height() const {
  return size_.height;
}

Size KChartWnd::GetSize() const { return {}; }

Point KChartWnd::GetLocation() const { return {}; }

Rect KChartWnd::GetBounds() const { return {}; }

std::string KChartWnd::GetTitle() const { return {}; }

void KChartWnd::SetSize(const Size& size) {

}

void KChartWnd::SetLocation(const Point& point) {

}

void KChartWnd::SetBounds(const Rect& rect) {
  ::MoveWindow(
    handle_, rect.left, rect.top,
    rect.Width(), rect.Height(), TRUE
  );
}

void KChartWnd::SetTitle(const std::string& str) {

}

void KChartWnd::Show(bool show) {
  ::ShowWindow(handle_, show ? SW_SHOW : SW_HIDE);
}

void KChartWnd::Invalidate() {
  // TODO: 发送重绘消息
  RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = (int)Width();
  rect.bottom = (int)Height();
  InvalidateRect(handle_, &rect, TRUE);
}

void KChartWnd::ChangeTheme(bool white) {
  if (white) {
    backColor_ = 0xFFFFFFFF; //0xFFD4D4D4
    borderColor_ = 0xFFE0E0E0;
    crosshairColor_ = 0xFF505050;

    for (int i = 0; i < (int)areas_.size(); ++i) {
      GraphArea* area = areas_[i];
      area->SetLabelBackColor(0xFFE0E0E0);
      area->SetScaleLineColor(0xFFEAEAEA);

      for (Graphics* g : area->graphics_) {
        g->ChangeTheme(white);
      }

      VerticalAxis* lvAxis = lvAxis_[i];
      VerticalAxis* rvAxis = rvAxis_[i];
      lvAxis->SetScaleColor(0xFF707070);
      lvAxis->SetCrosshairBackColor(0xFFE0E0E0);
      rvAxis->SetScaleColor(0xFF707070);
      rvAxis->SetCrosshairBackColor(0xFFE0E0E0);
      area->SetForceChange();
    }
    hAxis_->SetScaleColor(0xFF707070);
    hAxis_->SetCrosshairBackColor(0xFFE0E0E0);
    hAxis_->SetForceChange();
  }
  else {
    backColor_ = 0xFF2B2B2B;
    borderColor_ = 0xFF3F3F3F;
    crosshairColor_ = 0xFFAFAFAF;

    for (int i = 0; i < (int)areas_.size(); ++i) {
      GraphArea* area = areas_[i];
      area->SetLabelBackColor(0xFF3F3F3F);
      area->SetScaleLineColor(0xFF353535);

      for (Graphics* g : area->graphics_) {
        g->ChangeTheme(white);
      }

      VerticalAxis* lvAxis = lvAxis_[i];
      VerticalAxis* rvAxis = rvAxis_[i];
      lvAxis->SetScaleColor(0xFF8F8F8F);
      lvAxis->SetCrosshairBackColor(0xFF3F3F3F);
      rvAxis->SetScaleColor(0xFF8F8F8F);
      rvAxis->SetCrosshairBackColor(0xFF3F3F3F);
      area->SetForceChange();
    }
    hAxis_->SetScaleColor(0xFF8F8F8F);
    hAxis_->SetCrosshairBackColor(0xFF3F3F3F);
    hAxis_->SetForceChange();
  }
}

Rect KChartWnd::GetAreaBounds() {
  Rect bounds = {
      0, 0, Width(), Height()
  };

  if (lvVisible_) {
    bounds.left = vAxisWidth_;
  }
  if (rvVisible_)
    bounds.right -= vAxisWidth_;

  bounds.bottom -= hAxis_->GetHeight();

  return bounds;
}

GraphArea* KChartWnd::CreateArea(float weight) {
  auto* laxis = new VerticalAxis(vAxisWidth_, true);
  auto* raxis = new VerticalAxis(vAxisWidth_, false);

  GraphArea* area = new GraphArea(this, laxis, raxis);
  area->SetWeight(weight);
  areas_.push_back(area);

  lvAxis_.push_back(laxis);
  rvAxis_.push_back(raxis);

  Layout();

  area->OnFitIdx(beginIdx_, endIdx_);

  area->UpdateScales();
  return area;
}

void KChartWnd::SetHAxis(HorizontalAxis* axis) {
  if (axis == nullptr)
    return;

  delete hAxis_;

  hAxis_ = axis;
  hAxis_->OnFitIdx(beginIdx_, endIdx_);
}

HorizontalAxis* KChartWnd::GetHAxis() {
  return hAxis_;
}

void KChartWnd::SetFixedCount(int count) {
  fixedCount_ = count > 0;
  if (fixedCount_) {
    beginIdx_ = 0;
    endIdx_ = count;
    sWidth_ = 1;
  }

  for (GraphArea* area : areas_) {
    area->OnFitIdx(beginIdx_, endIdx_);
    area->UpdateScales();
  }

  hAxis_->OnFitIdx(beginIdx_, endIdx_);
}

void KChartWnd::Zoom(int factor) {
  if (fixedCount_ || factor == 0)
    return;

  float old = sWidth_;

  const float step[] = { 2, 1.5, 1 };

  // 这里看起来有点让人抓狂, 但这是有必要的, 为了保证
  // (像蜡烛图这种中间带有分割线的)图形左右对称, 以及缩放
  // 更流畅.
  if (sWidth_ < 1 || (sWidth_ == 1 && factor < 0)) {
    sWidth_ += float(factor) * 0.1f;
  }
  else if (sWidth_ < 2 || (sWidth_ == 2 && factor < 0)) {
    sWidth_ += float(factor) * 0.5f;
  }
  else {
    if (sWidth_ < 3 || (sWidth_ == 3 && factor < 0)) {
      sWidth_ += float(factor);
    }
    else {
      sWidth_ += float(factor * 2);
    }
  }
  if (sWidth_ <= 0)
    sWidth_ = 0.1f;

  // 由于浮点数存在精度问题, 所以只能通过这种方式判断相等.
  if (abs(old - sWidth_) < 1e-4) {
    return;
  }

  FitNewWidth();

  for (auto area : areas_)
    area->UpdateScales();
}

void KChartWnd::MoveCrosshair(int offset) {
  if (areas_.empty())
    return;

  GraphArea* area = areas_.front();

  int showCount = min(endIdx_ - beginIdx_, data_->RowCount());
  if (showCount <= 0)
    return;

  DrawData data(*data_, beginIdx_, showCount);
  FillDrawData(area, data);

  int idx;
  int cidx = area->GetCrosshairIndex();
  /*if (cidx) {
    idx = offset > 0 ? (data.Count() - 1) : 0;
    x = area->GetBounds().left + data.ToPX(idx);
  } else {*/
  idx = cidx + offset;
  if (idx < 0 || idx >= data.Count()) {
    if (offset < 0 && (beginIdx_ + offset) < 0)          return;
    if (offset > 0 && (endIdx_ + offset) > data_->RowCount()) return;
    idx = offset > 0 ? (data.Count() - 1) : 0;
    beginIdx_ += offset;
    endIdx_ += offset;

    for (auto* item : areas_) {
      item->OnFitIdx(beginIdx_, endIdx_);
      item->UpdateScales();
      item->SetForceChange();
    }
    hAxis_->OnFitIdx(beginIdx_, endIdx_);
    hAxis_->SetForceChange();
    return;
  }
  //}

  crosshairVisible_ = true;
  for (auto* item : areas_) {
    item->OnMoveCrosshair(idx);
  }
  hAxis_->OnMoveCrosshair(idx);
  // OnSetCrosshairPoint({x, crosshairPoint_.y});
}

void KChartWnd::FastScroll(int dir) {
  if (areas_.empty())
    return;

  crosshairVisible_ = false;

  // 是否到边界了?
  if ((dir < 0 && beginIdx_ == 0)
    || (dir > 0 && endIdx_ >= data_->RowCount())) {
    return;
  }

  int showCount = endIdx_ - beginIdx_;
  float off = float(dir) * float(showCount) * 0.1f;
  if ((dir > 0 && off < 1) || (dir < 0 && off > -1))
    off = (float)dir;
  beginIdx_ += (int)off;
  endIdx_ += (int)off;
  if (beginIdx_ < 0) {
    beginIdx_ = 0;
    endIdx_ = showCount;
  }
  else if (endIdx_ > data_->RowCount()) {
    beginIdx_ = data_->RowCount() - showCount;
    endIdx_ = data_->RowCount();
  }

  for (GraphArea* area : areas_) {
    area->OnFitIdx(beginIdx_, endIdx_);
    area->UpdateScales();
    area->SetForceChange();
  }

  hAxis_->OnFitIdx(beginIdx_, endIdx_);
  hAxis_->SetForceChange();
}

void KChartWnd::OnSetCrosshairPoint(Point point) {
  crosshairPoint_ = point;

  int count = (int)areas_.size();
  for (int i = 0; i < count; ++i) {
    GraphArea* item = areas_[i];
    Rect bounds = item->GetBounds();
    bounds.top += item->GetLabelHeight();

    Scalar width = bounds.right - bounds.left;

    Scalar x = crosshairPoint_.x - bounds.left;
    Scalar y = crosshairPoint_.y - bounds.top;
    x = max(-1, x);
    y = max(-1, y);
    if (x > width)
      x = -1;
    if (y > item->GetContentHeight())
      y = -1;

    item->OnMoveCrosshair({ x, y });
    lvAxis_[i]->OnMoveCrosshair({ x, y });
    rvAxis_[i]->OnMoveCrosshair({ x, y });
    if (i == count - 1) {
      hAxis_->OnMoveCrosshair({ x, -1 });
    }
  }
}

LRESULT KChartWnd::OnMessage(
  UINT msg, WPARAM wParam, LPARAM lParam
) {
  LRESULT nRes;
  switch (msg) {
  case WM_CREATE:
    nRes = OnProcCreate();
    break;
  case WM_SIZE:
    nRes = OnProcSize((Scalar)LOWORD(lParam), (Scalar)HIWORD(lParam));
    break;
  case WM_PAINT: {
    PAINTSTRUCT paint;
    BeginPaint(handle_, &paint);

    Rect rect = {
        paint.rcPaint.left, paint.rcPaint.top,
        paint.rcPaint.right, paint.rcPaint.bottom
    };

    gcContext_->AllocBuffer(rect.Size());
    nRes = OnProcPaint(rect);
    gcContext_->SwapBuffer(paint.hdc);

    EndPaint(handle_, &paint);
  }
               break;
  case WM_LBUTTONDOWN: {
    nRes = OnProcLBtnDown({ (Scalar)LOWORD(lParam), (Scalar)HIWORD(lParam) });
  }
                     break;
  case WM_MOUSEMOVE:
    nRes = OnProcMouseMove({ (Scalar)LOWORD(lParam), (Scalar)HIWORD(lParam) });
    break;
  case WM_DESTROY:
    handle_ = NULL; // no break

  default:
    return DefWindowProcA(handle_, msg, wParam, lParam);
  }

  if (nRes != 0)
    return nRes;

  return DefWindowProcA(handle_, msg, wParam, lParam);
}

LRESULT KChartWnd::OnProcCreate() {
  return 0;
}

LRESULT KChartWnd::OnProcSize(Scalar width, Scalar height) {
  if (width == size_.width
    && height == size_.height) {
    return 0;
  }

  bool widthChg = width != size_.width;

  size_.width = width;
  size_.height = height;

  Layout();

  if (widthChg)
    FitNewWidth();

  for (auto area : areas_)
    area->UpdateScales();

  Invalidate();

  return 0;
}

LRESULT KChartWnd::OnProcPaint(Rect rect) {
  Size size = { Scalar(rect.right - rect.left),
               Scalar(rect.bottom - rect.top) };

  gcContext_->SetTranslate({ 0 });

  // 填充背景
  gcContext_->SetColor(backColor_);

  gcContext_->FillRect(0, 0, size.width, size.height);

  // TODO:

  for (int i = 0; i < (int)areas_.size(); i++) {
    GraphArea* area = areas_[i];
    VerticalAxis* lvAxis = lvAxis_[i];
    VerticalAxis* rvAxis = rvAxis_[i];

    lvAxis->SetWidth(vAxisWidth_);
    rvAxis->SetWidth(vAxisWidth_);

    Rect bounds = area->GetBounds();
    Scalar offY = area->GetContentTop();

    int showCount = min(endIdx_ - beginIdx_, data_->RowCount());
    DrawData data(*data_, beginIdx_, showCount);
    FillDrawData(area, data);

    gcContext_->SetTranslate({ 0, bounds.top });

    if (lvVisible_) {
      lvAxis->OnPaint(gcContext_, data, offY);
      gcContext_->Translate({ vAxisWidth_, 0 });
    }

    area->OnPaint(gcContext_, data);

    if (rvVisible_) {
      gcContext_->Translate({ bounds.Width(), 0 });
      rvAxis->OnPaint(gcContext_, data, offY);
    }

    // 横轴线
    gcContext_->SetTranslate({ 0 });
    gcContext_->SetColor(borderColor_);
    gcContext_->DrawLine(
      bounds.left, bounds.bottom,
      bounds.right, bounds.bottom
    );

    if (i == (int)areas_.size() - 1) {
      gcContext_->SetTranslate({ bounds.left, bounds.bottom });
      hAxis_->OnPaint(gcContext_, data, bounds.Width());
    }
  }

  gcContext_->SetTranslate({ 0 });

  // 纵轴线
  Rect bounds = GetAreaBounds();
  gcContext_->SetColor(borderColor_);
  if (lvVisible_) {
    gcContext_->DrawLine(
      bounds.left, bounds.top,
      bounds.left, bounds.bottom
    );
  }
  if (rvVisible_) {
    gcContext_->DrawLine(
      bounds.right, bounds.top,
      bounds.right, bounds.bottom
    );
  }

  return 0;
}

LRESULT KChartWnd::OnProcLBtnDown(Point point) {
  if (!crosshairEnable_)
    return 1;

  crosshairVisible_ = !crosshairVisible_;
  if (crosshairVisible_)
    OnSetCrosshairPoint(point);
  else
    OnSetCrosshairPoint({ -1, -1 });

  Invalidate();
  return 0;
}

LRESULT KChartWnd::OnProcMouseMove(Point point) {
  if (crosshairVisible_) {
    OnSetCrosshairPoint(point);
    Invalidate();
    return 0;
  }

  return 1;
}

void KChartWnd::FillDrawData(GraphArea* area, DrawData& data) {
  Rect bounds = area->GetBounds();

  int wCount = endIdx_ - beginIdx_;
  DataType hCount = area->GetMax() - area->GetMin();
  Size drawSize = {
      bounds.Width(),
      area->GetContentHeight()
  };

  data.size = drawSize;
  data.bias = area->GetMin();
  data.wRatio = float(drawSize.width) / float(wCount);
  data.hRatio = float(drawSize.height) / float(hCount);
  data.sWidth = int(sWidth_);
  if (data.sWidth < 3)
    data.sWidth = 1;
  data.sMargin = data.sWidth / 4;
}

void KChartWnd::Layout() {
  float weightSum = 0;
  for (auto& area : areas_) {
    weightSum += area->GetWeight();
  }

  float height = float(Height() - hAxis_->GetHeight());
  Rect bounds = GetAreaBounds();
  bounds.bottom = bounds.top;

  for (auto& area : areas_) {
    float ratio = area->GetWeight() / weightSum;
    bounds.bottom += Scalar(ratio * height);
    area->SetBounds(bounds);
    bounds.top = bounds.bottom;
  }
}

void KChartWnd::FitNewWidth() {
  Rect rect = GetAreaBounds();
  Scalar width = rect.right - rect.left;

  if (fixedCount_) {
    //sWidth_ = float(width) / float(endIdx_);
    return;
  }

  int newCount = int(float(width) / sWidth_);
  if (newCount > 0) {
    if (newCount > data_->RowCount()) {
      beginIdx_ = 0;
      endIdx_ = newCount;
    }
    else {
      if (endIdx_ == 0 || endIdx_ > data_->RowCount()) {
        if (data_->RowCount() > newCount)
          endIdx_ = data_->RowCount();
        else
          endIdx_ = newCount;
      }

      beginIdx_ = endIdx_ - newCount;
      if (beginIdx_ < 0) {
        beginIdx_ = 0;
        endIdx_ = newCount;
      }
    }
  }

  for (const auto& item : areas_) {
    item->OnFitIdx(beginIdx_, endIdx_);
  }

  hAxis_->OnFitIdx(beginIdx_, endIdx_);
}

} /* namespace kchart */
