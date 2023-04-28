//
// Created by luo-zeqi on 2013/4/19.
//

#ifndef LIBKCHART_GDIPLUSGC_H
#define LIBKCHART_GDIPLUSGC_H

#include "GraphContext.h"

#include <map>
#include <Windows.h>
#include <gdiplus.h>

namespace kchart {

class GdiPlusGC : public GraphContext
{
public:
    GdiPlusGC();
    virtual ~GdiPlusGC();

    void AllocBuffer(const Size& size);
    void SwapBuffer(HDC dc);

public:
    Point Translate(const Point& point) override;
    void SetTranslate(const Point& point) override;

    void SetFont(FontId fontId, int size) override;
    void SetColor(Color color) override;

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
    void AddPolyLine(const Point& p) override;
    void EndPolyLine() override;

    void DrawStr(const WStr& str, const Point &p) override;
    Size MeasureStr(const WStr& str) override;

private:
    HDC     memDC_;
    HBITMAP memBitmap_;

    Color color_;

    Gdiplus::Graphics   *gdiGraph_;
    Gdiplus::Pen        *gdiPen_;
    Gdiplus::SolidBrush *gdiBrush_;
    Gdiplus::Font       *gdiFont_;

    int pointNum_;
    std::vector<Gdiplus::Point> pointCache_;

    Size cacheSize_;

    Point offset_;

    std::map<uint64_t, Gdiplus::Font*> fontCache_;
};

}

#endif //LIBKCHART_GDIPLUSGC_H
