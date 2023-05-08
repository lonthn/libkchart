//
// Created by luo.zeqi on 2013/4/13.
//

#ifndef WINCTRL_KLINEWND_H
#define WINCTRL_KLINEWND_H

#include "Str.h"

#include "GraphArea.h"
#include "Graphics.h"
#include "Struct.h"
#include "graph/GdiPlusGC.h"
#include "VerticalAxis.h"

#include <Windows.h>

class WndProcThunk;

namespace kchart {

class KChartWnd
{
public:
    KChartWnd();

    HWND Handle() {
        return handle_;
    }

    DataSet& DataRef() {
        return data_;
    }

    virtual bool CreateWin(HWND hParent = nullptr);

    virtual Scalar Width() const;
    virtual Scalar Height() const;
    virtual Size   GetSize() const;
    virtual Point  GetLocation() const;
    virtual Rect   GetBounds() const;
    virtual Str    GetTitle() const;

    virtual void SetSize(const Size& size);
    virtual void SetLocation(const Point& point);
    virtual void SetBounds(const Rect& rect);
    virtual void SetTitle(const Str& str);

    virtual void Show(bool show);
    virtual void Invalidate();

    virtual Rect GetAreaBounds();

    virtual GraphArea *CreateArea(float percent);

    virtual void Zoom(int factor);

public:
    static Str ClassName() {
        return "WINEX_CLS_KChart";
    }

public:
    LRESULT CALLBACK OnMessage(
        UINT msg, WPARAM wParam, LPARAM lParam
    );

    virtual LRESULT OnCreate (WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnSize   (WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnPaint  (WPARAM wParam, LPARAM lParam);

private:
    void Layout();
    void FitNewWidth();

private:
    HWND handle_;

    Color borderColor_;
    Size size_;

    bool lvVisible_;
    bool rvVisible_;
    Scalar vAxisWidth_;
    std::vector<VerticalAxis*> lvAxis_;
    std::vector<VerticalAxis*> rvAxis_;
    std::vector<GraphArea*> areas_;

    int beginIdx_;
    int endIdx_;
    int sWidth_;
    DataSet data_;

    WndProcThunk *procThunk_;
    GdiPlusGC *gcContext_;
};

}

#endif // WINCTRL_KLINEWND_H
