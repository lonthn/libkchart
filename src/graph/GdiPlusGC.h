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

#ifndef LIBKCHART_GDIPLUSGC_H
#define LIBKCHART_GDIPLUSGC_H

#include "graph/GraphContext.h"

#include <map>
#include <Windows.h>
#include <gdiplus.h>

namespace kchart {

class GdiPlusGC : public GraphContext {
public:
  GdiPlusGC();

  virtual ~GdiPlusGC();

  void AllocBuffer(const Size &size);
  void SwapBuffer(HDC dc);

public:
  Point Translate(const Point &point) override;
  void SetTranslate(const Point &point) override;
  void SetFont(FontId fontId, int size) override;
  void SetColor(Color color) override;
  void SetLineWidth(int width) override;

  void DrawRect(const Rect &r) override;
  void DrawRect(
      Scalar l, Scalar t, Scalar r, Scalar b
  ) override;

  void FillRect(const Rect &r) override;
  void FillRect(
      Scalar l, Scalar t, Scalar r, Scalar b
  ) override;

  void DrawLine(
      const Point &a, const Point &b
  ) override;
  void DrawLine(
      Scalar ax, Scalar ay, Scalar bx, Scalar by
  ) override;

  void BeginPolyLine(int pointNum) override;
  void AddPolyLine(const Point &p) override;
  void EndPolyLine() override;

  void DrawStr(const CStringW &str, const Point &p) override;
  Size MeasureStr(const CStringW &str) override;

private:
  HDC memDC_;
  HBITMAP memBitmap_;

  Color color_;

  Gdiplus::Graphics *gdiGraph_;
  Gdiplus::Pen *gdiPen_;
  Gdiplus::SolidBrush *gdiBrush_;
  Gdiplus::Font *gdiFont_;

  int pointNum_;
  std::vector<Gdiplus::Point> pointCache_;

  Size cacheSize_;

  Point offset_;

  std::map<uint64_t, Gdiplus::Font *> fontCache_;
};

}

#endif //LIBKCHART_GDIPLUSGC_H
