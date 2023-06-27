//
// Created by luo-zeqi on 2013/4/20.
//

#include "GdiPlusGC.h"

#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

namespace kchart {

ULONG_PTR GdiPlusToken = 0;

static HBITMAP CreateBitmap(HDC dc, int width, int height, void **bytes) {
  BITMAPINFO bmi = {0};
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
    : memDC_(NULL), memBitmap_(NULL), cacheSize_({0}), offset_({0}) {
  if (GdiPlusToken == 0) {
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&GdiPlusToken, &input, NULL);
  }

  memDC_ = CreateCompatibleDC(NULL);
  gdiGraph_ = NULL;
  gdiPen_ = new Gdiplus::Pen(Gdiplus::Color::White);
  gdiBrush_ = new Gdiplus::SolidBrush(Gdiplus::Color::White);

  SetFont(FontId_WRYH, 8);
}

GdiPlusGC::~GdiPlusGC() {
  delete gdiBrush_;
  delete gdiPen_;

  if (memBitmap_ != NULL)
    DeleteObject(memBitmap_);

  DeleteDC(memDC_);
}

void GdiPlusGC::AllocBuffer(const Size &size) {
  if (memBitmap_ == NULL
      || size.width > cacheSize_.width
      || size.height > cacheSize_.height) {
//        memBitmap_ = CreateCompatibleBitmap(
//            memDC_,
//            (int) size.width,
//            (int) size.height
//        );
    void *bytes = NULL;
    memBitmap_ = CreateBitmap(memDC_,
                              (int) size.width,
                              (int) size.height,
                              &bytes);

    HGDIOBJ oldBitmap = SelectObject(memDC_, memBitmap_);

    if (oldBitmap != NULL)
      DeleteObject(oldBitmap);

    if (gdiGraph_ != NULL) {
      delete gdiGraph_;
    }

    gdiGraph_ = new Gdiplus::Graphics(memDC_);
  }

  cacheSize_ = size;
}

void GdiPlusGC::SwapBuffer(HDC dc) {
  int cx = (int) cacheSize_.width;
  int cy = (int) cacheSize_.height;
  ::BitBlt(dc, 0, 0, cx, cy, memDC_, 0, 0, SRCCOPY);
}

Point GdiPlusGC::Translate(const Point &point) {
  Point oldPoint = offset_;
  offset_.x += point.x;
  offset_.y += point.y;
  return oldPoint;
}

void GdiPlusGC::SetTranslate(const Point &point) {
  offset_ = point;
}

void GdiPlusGC::SetFont(FontId fontId, int size) {
  uint64_t key = (uint64_t(fontId) << 32) | uint64_t(size);
  Gdiplus::Font *&font = fontCache_[key];
  if (font == NULL)
    font = new Gdiplus::Font(
        fontFamilies_[FontId_WRYH].GetString(),
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
  gdiPen_->SetWidth(width);
}

void GdiPlusGC::DrawRect(const Rect &r) {
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

void GdiPlusGC::FillRect(const Rect &r) {
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

void GdiPlusGC::DrawLine(const Point &a, const Point &b) {
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
  if ((int) pointCache_.size() < pointNum) {
    pointCache_.resize(pointNum);
  }
  pointNum_ = 0;
}

void GdiPlusGC::AddPolyLine(const Point &p) {
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


void GdiPlusGC::DrawStr(const CStringW &str, const Point &p) {
//    Gdiplus::GraphicsPath graphicsPathObj;
//    Gdiplus::FontFamily fontfamily;
//    font.GetFamily(&fontfamily);
//    graphicsPathObj.AddString(szText,-1,&fontfamily,font.GetStyle(),font.GetSize(),\
//                              PointF(0,0),&strFormat);

  Gdiplus::PointF location(Gdiplus::REAL(p.x + offset_.x),
                           Gdiplus::REAL(p.y + offset_.y));
  gdiGraph_->DrawString(str.GetString(), str.GetLength(),
                        gdiFont_, location, gdiBrush_);
}

Size GdiPlusGC::MeasureStr(const CStringW &str) {
  Gdiplus::StringFormat format(Gdiplus::StringAlignmentNear);

  // 接收字体的显示区域，如宽高
  Gdiplus::RectF stringRect;
  Gdiplus::RectF layoutRect(0, 0, 600, 100);
  // gdiGraph_->SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit );

  //获得字体高度与宽度stringRect
  gdiGraph_->MeasureString(str.GetString(), str.GetLength(),
                           gdiFont_, layoutRect,
                           &format, &stringRect);
  return {Scalar(stringRect.Width), Scalar(stringRect.Height)};
}

}