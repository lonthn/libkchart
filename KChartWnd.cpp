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
    , fixedCount_(false)
    , crosshairEnable_(true)
    , crosshairVisible_(false)
    , crosshairColor_(0xFFFFFFFF)
    , crosshairPoint_({-1, -1})
    , lvVisible_(true)
    , rvVisible_(true)
    , vAxisWidth_(80)
    , borderColor_(0xFF3F3F3F)
    , backColor_(0xFF2B2B2B) {
  union {
    LRESULT (KCALLBACK KChartWnd::*proc)(UINT, WPARAM, LPARAM);

    uintptr_t procPtr;
  } u{};

  u.proc = &KChartWnd::OnMessage;

  procThunk_ = new WndProcThunk();
  procThunk_->init((uintptr_t) this, u.procPtr);

  gcContext_ = new GdiPlusGC();

  hAxis_ = new HorizontalAxis(this);

  rowCount_ = data_.RowCount();
  data_.AddObserver([&](const Index2& idx) {
    if (data_.RowCount() == rowCount_)
      return;
    rowCount_ = data_.RowCount();
    hAxis_->OnFitIdx(beginIdx_, endIdx_);
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

void KChartWnd::SetSize(const Size &size) {

}

void KChartWnd::SetLocation(const Point &point) {

}

void KChartWnd::SetBounds(const Rect &rect) {

}

void KChartWnd::SetTitle(const std::string &str) {

}

void KChartWnd::Show(bool show) {
  ::ShowWindow(handle_, show ? SW_SHOW : SW_HIDE);
}

void KChartWnd::Invalidate() {
  // TODO: 发送重绘消息
  RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = (int) Width();
  rect.bottom = (int) Height();
  InvalidateRect(handle_, &rect, TRUE);
}

void KChartWnd::ChangeTheme(bool white) {
  if (white) {
    backColor_   = 0xFFFFFFFF; //0xFFD4D4D4
    borderColor_ = 0xFFE0E0E0;
    crosshairColor_ = 0xFF505050;

    for (int i = 0; i < (int) areas_.size(); ++i) {
      GraphArea *area = areas_[i];
      area->SetLabelBackColor(0xFFE0E0E0);
      area->SetScaleLineColor(0xFFEAEAEA);

      for (Graphics *g : area->graphics_) {
        g->ChangeTheme(white);
      }

      VerticalAxis *lvAxis = lvAxis_[i];
      VerticalAxis *rvAxis = rvAxis_[i];
      lvAxis->SetScaleColor(0xFF707070);
      lvAxis->SetCrosshairBackColor(0xFFE0E0E0);
      rvAxis->SetScaleColor(0xFF707070);
      rvAxis->SetCrosshairBackColor(0xFFE0E0E0);
      area->SetForceChange();
    }
    hAxis_->SetScaleColor(0xFF707070);
    hAxis_->SetCrosshairBackColor(0xFFE0E0E0);
    hAxis_->SetForceChange();
  } else {
    backColor_   = 0xFF2B2B2B;
    borderColor_ = 0xFF3F3F3F;
    crosshairColor_ = 0xFFAFAFAF;

    for (int i = 0; i < (int) areas_.size(); ++i) {
      GraphArea *area = areas_[i];
      area->SetLabelBackColor(0xFF3F3F3F);
      area->SetScaleLineColor(0xFF353535);

      for (Graphics *g : area->graphics_) {
        g->ChangeTheme(white);
      }

      VerticalAxis *lvAxis = lvAxis_[i];
      VerticalAxis *rvAxis = rvAxis_[i];
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

GraphArea *KChartWnd::CreateArea(float weight) {
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

void KChartWnd::SetHAxis(HorizontalAxis *axis) {
  if (axis == nullptr)
    return;

  delete hAxis_;

  hAxis_ = axis;
  hAxis_->OnFitIdx(beginIdx_, endIdx_);
}

HorizontalAxis *KChartWnd::GetHAxis() {
  return hAxis_;
}

void KChartWnd::SetFixedCount(int count) {
  fixedCount_ = count > 0;
  if (fixedCount_) {
    beginIdx_ = 0;
    endIdx_ = count;
    sWidth_ = 1;
  }

  for (GraphArea *area: areas_) {
    area->OnFitIdx(beginIdx_, endIdx_);
    area->UpdateScales();
  }

  hAxis_->OnFitIdx(beginIdx_, endIdx_);
}

void KChartWnd::Zoom(int factor) {
  if (fixedCount_ || factor == 0)
    return;

  float old = sWidth_;

  const float step[] = {2, 1.5, 1};

  // 这里看起来有点让人抓狂, 但这是有必要的, 为了保证
  // (像蜡烛图这种中间带有分割线的)图形左右对称, 以及缩放
  // 更流畅.
  if (sWidth_ < 1 || (sWidth_ == 1 && factor < 0)) {
    sWidth_ += float(factor) * 0.1f;
  } else if (sWidth_ < 2 || (sWidth_ == 2 && factor < 0)) {
    sWidth_ += float(factor) * 0.5f;
  } else {
    if (sWidth_ < 3 || (sWidth_ == 3 && factor < 0)) {
      sWidth_ += float(factor);
    } else {
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

  for (auto area: areas_)
    area->UpdateScales();
}

void KChartWnd::MoveCrosshair(int offset) {
  if (areas_.empty())
    return;

  GraphArea *area = areas_.front();

  int showCount = min(endIdx_ - beginIdx_, data_.RowCount());
  if (showCount <= 0)
    return;

  DrawData data(data_, beginIdx_, showCount);
  FillDrawData(area, data);

  int idx;
  int cidx = area->GetCrosshairIndex();
  /*if (cidx) {
    idx = offset > 0 ? (data.Count() - 1) : 0;
    x = area->GetBounds().left + data.ToPX(idx);
  } else {*/
  idx = cidx + offset;
  if (idx < 0 || idx >= data.Count()) {
  	if (offset < 0 && (beginIdx_+offset) < 0)          return;
  	if (offset > 0 && (endIdx_+offset) > data_.RowCount()) return;
  	idx = offset > 0 ? (data.Count() - 1) : 0;
  	beginIdx_ += offset;
  	endIdx_ += offset;

  	for (auto *item: areas_) {
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
  for (auto *item : areas_) {
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
   || (dir > 0 && endIdx_ >= data_.RowCount())) {
    return;
  }

  int showCount = endIdx_ - beginIdx_;
  float off = float(dir) * float(showCount) * 0.1f;
  if ((dir > 0 && off < 1) || (dir < 0 && off > -1))
    off = (float) dir;
  beginIdx_ += (int) off;
  endIdx_ += (int) off;
  if (beginIdx_ < 0) {
    beginIdx_ = 0;
    endIdx_ = showCount;
  } else if (endIdx_ > data_.RowCount()) {
    beginIdx_ = data_.RowCount() - showCount;
    endIdx_ = data_.RowCount();
  }

  for (GraphArea *area: areas_) {
    area->OnFitIdx(beginIdx_, endIdx_);
    area->UpdateScales();
    area->SetForceChange();
  }

  hAxis_->OnFitIdx(beginIdx_, endIdx_);
  hAxis_->SetForceChange();
}

void KChartWnd::OnSetCrosshairPoint(Point point) {
  crosshairPoint_ = point;

  int count = (int) areas_.size();
  for (int i = 0; i < count; ++i) {
    GraphArea *item = areas_[i];
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
    lvAxis_[i]->OnMoveCrosshair({x, y});
    rvAxis_[i]->OnMoveCrosshair({x, y});
    if (i == count - 1) {
      hAxis_->OnMoveCrosshair({x, -1});
    }
  }
}

LRESULT KChartWnd::OnMessage(
    UINT msg, WPARAM wParam, LPARAM lParam
) {
  LRESULT nRes;
  switch (msg) {
    case WM_CREATE:
      nRes = OnCreate(wParam, lParam);
      break;
    case WM_SIZE:
      nRes = OnSize(wParam, lParam);
      break;
    case WM_PAINT:
      nRes = OnPaint(wParam, lParam);
      break;
    case WM_LBUTTONDOWN:
      nRes = OnLBtnDown(wParam, lParam);
      break;
    case WM_MOUSEMOVE:
      nRes = OnMouseMove(wParam, lParam);
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

LRESULT KChartWnd::OnCreate(WPARAM wParam, LPARAM lParam) {
  return 0;
}

LRESULT KChartWnd::OnSize(WPARAM wParam, LPARAM lParam) {
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

  for (auto area: areas_)
    area->UpdateScales();

  Invalidate();

  return 0;
}

LRESULT KChartWnd::OnPaint(WPARAM wParam, LPARAM lParam) {
  PAINTSTRUCT paint;
  BeginPaint(handle_, &paint);

  RECT rect = paint.rcPaint;
  Size size = {Scalar(rect.right - rect.left),
               Scalar(rect.bottom - rect.top)};

  gcContext_->AllocBuffer(size);

  gcContext_->SetTranslate({0});

  // 填充背景
  gcContext_->SetColor(backColor_);

  gcContext_->FillRect(0, 0, size.width, size.height);

  // TODO:

  for (int i = 0; i < (int) areas_.size(); i++) {
    GraphArea *area = areas_[i];
    VerticalAxis *lvAxis = lvAxis_[i];
    VerticalAxis *rvAxis = rvAxis_[i];

    Rect bounds = area->GetBounds();
    Scalar offY = area->GetContentTop();

    int showCount = min(endIdx_ - beginIdx_, data_.RowCount());
    DrawData data(data_, beginIdx_, showCount);
    FillDrawData(area, data);

    gcContext_->SetTranslate({0, bounds.top});

    if (lvVisible_) {
      lvAxis->OnPaint(gcContext_, data, offY);
      gcContext_->Translate({vAxisWidth_, 0});
    }

    area->OnPaint(gcContext_, data);

    if (rvVisible_) {
      gcContext_->Translate({bounds.Width(), 0});
      rvAxis->OnPaint(gcContext_, data, offY);
    }

    // 横轴线
    gcContext_->SetTranslate({0});
    gcContext_->SetColor(borderColor_);
    gcContext_->DrawLine(
        bounds.left, bounds.bottom,
        bounds.right, bounds.bottom
    );

    if (i == (int) areas_.size() - 1) {
      gcContext_->SetTranslate({bounds.left, bounds.bottom});
      hAxis_->OnPaint(gcContext_, data, bounds.Width());
    }
  }

  gcContext_->SetTranslate({0});

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

  gcContext_->SwapBuffer(paint.hdc);

  EndPaint(handle_, &paint);

  return 0;
}

LRESULT KChartWnd::OnLBtnDown(WPARAM wParam, LPARAM lParam) {
  if (!crosshairEnable_)
    return 1;

  crosshairVisible_ = !crosshairVisible_;
  if (crosshairVisible_)
    OnSetCrosshairPoint({LOWORD(lParam), HIWORD(lParam)});
  else
    OnSetCrosshairPoint({-1, -1});

  Invalidate();
  return 0;
}

LRESULT KChartWnd::OnMouseMove(WPARAM wParam, LPARAM lParam) {
  if (crosshairVisible_) {
    OnSetCrosshairPoint({LOWORD(lParam), HIWORD(lParam)});
    Invalidate();
    return 0;
  }

  return 1;
}

void KChartWnd::FillDrawData(GraphArea *area, DrawData &data) {
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
  for (auto &area: areas_) {
    weightSum += area->GetWeight();
  }

  float height = float(Height() - hAxis_->GetHeight());
  Rect bounds = GetAreaBounds();
  bounds.bottom = bounds.top;

  for (auto &area: areas_) {
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
    if (newCount > data_.RowCount()) {
      beginIdx_ = 0;
      endIdx_ = newCount;
    } else {
      if (endIdx_ == 0 || endIdx_ > data_.RowCount()) {
        if (data_.RowCount() > newCount)
          endIdx_ = data_.RowCount();
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

  for (const auto &item: areas_) {
    item->OnFitIdx(beginIdx_, endIdx_);
  }

  hAxis_->OnFitIdx(beginIdx_, endIdx_);
}

} // namespace kchart