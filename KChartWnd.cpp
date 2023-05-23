//
// Created by luo-zeqi on 2013/4/13.
//

#include "KChartWnd.h"
#include "WndProcThunk.h"

#include "graph/GdiPlusGC.h"

#include <WinUser.h>

namespace kchart {

static bool GVClassReg = true;

KChartWnd::KChartWnd()
  : handle_(NULL)
  , procThunk_(NULL)
  , beginIdx_(0)
  , endIdx_(0)
  , size_({0})
  , sWidth_(5)
  , crosshairEnable_(true)
  , crosshairVisible_(false)
  , crosshairColor_(0xFFFFFFFF)
  , crosshairPoint_({-1, -1})
  , lvVisible_(true)
  , rvVisible_(true)
  , vAxisWidth_(80)
  , borderColor_(0xFF505050)
{
    union
    {
        LRESULT    (KCALLBACK KChartWnd::*proc)(UINT, WPARAM, LPARAM);
        uintptr_t  procPtr;
    } u {};

    u.proc = &KChartWnd::OnMessage;

    procThunk_ = new WndProcThunk();
    procThunk_->init((uintptr_t)this, u.procPtr);

    gcContext_ = new GdiPlusGC();
}

KChartWnd::~KChartWnd()
{
    if (gcContext_ != NULL)
    {
        delete gcContext_;
        gcContext_ = NULL;
    }
    if (procThunk_ != NULL)
    {
        delete procThunk_;
        procThunk_ = NULL;
    }

    for (int i = 0; i < areas_.size(); i++)
    {
        delete areas_[i];
        delete lvAxis_[i];
        delete rvAxis_[i];
    }
    areas_.clear();
    lvAxis_.clear();
    rvAxis_.clear();
}

bool KChartWnd::CreateWin(HWND hParent)
{
    WNDCLASSA cls;

    Str  className = ClassName();
    Str  wndName   = "KChartWnd";
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    if (GVClassReg)
    {
        cls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        cls.hInstance = hInstance;
        cls.lpszClassName = (char *) className.c_str();
        cls.lpszMenuName = NULL;
        cls.lpfnWndProc = DefWindowProcA;
        cls.cbClsExtra = 0;
        cls.cbWndExtra = 0;
        cls.hIcon = NULL;
        cls.hCursor = NULL;
        cls.hbrBackground = NULL; // CreateSolidBrush(0xFFFFFF);

        assert(RegisterClassA(&cls));

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

    return true;
}

Scalar KChartWnd::Width() const
{
    return size_.width;
}

Scalar KChartWnd::Height() const
{
    return size_.height;
}

Size   KChartWnd::GetSize() const { return {}; }
Point  KChartWnd::GetLocation() const { return {}; }
Rect   KChartWnd::GetBounds() const { return {}; }
Str KChartWnd::GetTitle() const { return ""; }

void KChartWnd::SetSize(const Size& size)
{

}
void KChartWnd::SetLocation(const Point& point)
{

}

void KChartWnd::SetBounds(const Rect& rect)
{

}

void KChartWnd::SetTitle(const Str& str)
{

}

void KChartWnd::Show(bool show)
{
    ::ShowWindow(handle_, show ? SW_SHOW : SW_HIDE);
}

void KChartWnd::Invalidate()
{
    // TODO: 发送重绘消息
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = (int) Width();
    rect.bottom = (int) Height();
    InvalidateRect(handle_, &rect, TRUE);
}

Rect KChartWnd::GetAreaBounds()
{
    Rect bounds = {
        0, 0, Width(), Height()
    };

    if (lvVisible_) {
        bounds.left = vAxisWidth_;
    }
    if (rvVisible_)
        bounds.right -= vAxisWidth_;

    return bounds;
}

GraphArea *KChartWnd::CreateArea(float weight)
{
    auto *laxis = new VerticalAxis(vAxisWidth_, true);
    auto *raxis = new VerticalAxis(vAxisWidth_, false);

    GraphArea *area = new GraphArea(this, laxis, raxis);
    area->SetWeight(weight);
    areas_.push_back(area);

    lvAxis_.push_back(laxis);
    rvAxis_.push_back(raxis);

    Layout();

    area->OnFitIdx(beginIdx_, endIdx_);

    area->UpdateScales();
    return area;
}

void KChartWnd::Zoom(int factor)
{
    sWidth_ += factor * 2;
    if (sWidth_ < 0)
        sWidth_ = 1;

    FitNewWidth();

    for (auto area : areas_)
        area->UpdateScales();
}

void KChartWnd::OnSetCrosshairPoint(Point point)
{
    crosshairPoint_ = point;

    for (auto &item: areas_)
    {
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

        item->OnMoveCrosshair({x, y});
    }
}

LRESULT KChartWnd::OnMessage(
    UINT msg, WPARAM wParam, LPARAM lParam
)
{
    LRESULT nRes;
    switch (msg) {
        case WM_CREATE:
            nRes = OnCreate(wParam, lParam); break;
        case WM_SIZE:
            nRes = OnSize(wParam, lParam);   break;
        case WM_PAINT:
            nRes = OnPaint(wParam, lParam);  break;
        case WM_LBUTTONDOWN:
            nRes = OnLBtnDown(wParam, lParam); break;
        case WM_MOUSEMOVE:
            nRes = OnMouseMove(wParam, lParam); break;
        case WM_DESTROY:
            handle_ = NULL; // no break

        default:
            return DefWindowProcA(handle_, msg, wParam, lParam);
    }

    if (nRes != 0)
        return nRes;

    return DefWindowProcA(handle_, msg, wParam, lParam);
}

LRESULT KChartWnd::OnCreate(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT KChartWnd::OnSize(WPARAM wParam, LPARAM lParam)
{
    Scalar width = Scalar(LOWORD(lParam));
    Scalar height = Scalar(HIWORD(lParam));

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

LRESULT KChartWnd::OnPaint(WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paint;
    BeginPaint(handle_, &paint);

    RECT rect = paint.rcPaint;
    Size size = {Scalar(rect.right-rect.left),
                 Scalar(rect.bottom-rect.top)};

    gcContext_->AllocBuffer(size);

    gcContext_->SetTranslate({0});

    // 填充背景
    gcContext_->SetColor(0xFF1C1F26);
    gcContext_->FillRect(0, 0, size.width, size.height);

    // TODO:

    for (int i = 0; i < (int) areas_.size(); i++)
    {
        GraphArea *area = areas_[i];
        Rect bounds = area->GetBounds();
        gcContext_->SetTranslate({bounds.left, bounds.top});
        area->OnPaint(gcContext_);
        gcContext_->SetTranslate({0, 0});

        if (i != areas_.size() - 1)
        {
            gcContext_->SetColor(borderColor_);
            gcContext_->DrawLine(bounds.left, bounds.bottom,
                                 bounds.right, bounds.bottom);
        }
    }

    gcContext_->SetColor(borderColor_);
    gcContext_->DrawLine(vAxisWidth_, 0,
                         vAxisWidth_, Height());
    gcContext_->DrawLine(Width() - vAxisWidth_, 0,
                         Width() - vAxisWidth_, Height());

    gcContext_->SwapBuffer(paint.hdc);

    EndPaint(handle_, &paint);

    return 0;
}

LRESULT KChartWnd::OnLBtnDown(WPARAM wParam, LPARAM lParam)
{
    if (!crosshairEnable_)
        return 1;

    crosshairVisible_ = !crosshairVisible_;
    if (crosshairVisible_)
    {
        OnSetCrosshairPoint({LOWORD(lParam), HIWORD(lParam)});
    }
    else
    {
        OnSetCrosshairPoint({-1, -1});
    }


    Invalidate();
    return 0;
}

LRESULT KChartWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (crosshairVisible_)
    {
        OnSetCrosshairPoint({LOWORD(lParam), HIWORD(lParam)});
        Invalidate();
        return 0;
    }

    return 1;
}

void KChartWnd::Layout()
{
    float weightSum = 0;
    for (auto & area : areas_) {
        weightSum += area->GetWeight();
    }

    Rect bounds = GetAreaBounds();

    bounds.bottom = bounds.top;

    for (auto & area : areas_) {
        float ratio = area->GetWeight() / weightSum;
        bounds.bottom += Scalar(ratio * float(Height()));
        area->SetBounds(bounds);
        bounds.top = bounds.bottom;
    }
}

void KChartWnd::FitNewWidth()
{
    Rect rect = GetAreaBounds();
    Scalar width = rect.right - rect.left;

    int newCount = int(float(width) / float(sWidth_));
    if (newCount > 0)
    {
        if (newCount > data_.RowCount())
        {
            endIdx_ = newCount;
            beginIdx_ = 0;
        }
        else
        {
            if (endIdx_ > data_.RowCount() || endIdx_ == 0)
            {
                endIdx_ = data_.RowCount();
            }
            beginIdx_ = endIdx_ - newCount;
        }
    }

    for (const auto &item: areas_) {
        item->OnFitIdx(beginIdx_, endIdx_);
    }
}

} // namespace kchart