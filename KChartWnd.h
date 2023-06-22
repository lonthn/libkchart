//
// Created by luo.zeqi on 2013/4/13.
//

#ifndef WINCTRL_KLINEWND_H
#define WINCTRL_KLINEWND_H

#include "GraphArea.h"
#include "Graphics.h"
#include "Struct.h"
#include "graph/GdiPlusGC.h"
#include "VerticalAxis.h"

#include <Windows.h>

class WndProcThunk;

namespace kchart {

/// @brief 容纳图表的窗体类, 可作为独立窗口运行, 也可以是
/// 窗体中的一个控件.
class KChartWnd {
public:
  KChartWnd();
  virtual ~KChartWnd();

  HWND Handle() {
      return handle_;
  }
  DataSet &DataRef() {
      return data_;
  }
  void SetCrosshairColor(Color color) {
      crosshairColor_ = color;
  }
  Color GetCrosshairColor() const {
      return crosshairColor_;
  }

  /// @brief 必须调用的函数, 使用WindowsAPI创建窗体
  /// hParent 即父窗体的句柄, 可为空.
  bool CreateWin(HWND hParent = nullptr);

  Scalar Width() const;
  Scalar Height() const;
  Size GetSize() const;
  Point GetLocation() const;
  Rect GetBounds() const;
  std::string GetTitle() const;
  void SetSize(const Size &size);
  void SetLocation(const Point &point);
  void SetBounds(const Rect &rect);
  void SetTitle(const std::string &str);

  void Show(bool show);
  /// @brief 使界面内容失效, 从而触发重绘事件
  /// 若使界面发生了变化, 必须要调用此函数进行重绘.
  void Invalidate();

  Rect GetAreaBounds();

  /// @brief 创建图形区域, 可以创建多个(上下结构),
  /// 每个区域都是独立的坐标系, percent 可以指定高
  /// 度权重(内部根据权重分配高度).
  virtual GraphArea *CreateArea(float percent);
  /**
   * @brief 设置将界面展示的数据条数固定
   * 设置后将无法使用 Zoom 和 MoveIdx
   * @param count 大于零即固定条数，相反则取消固定
   */
  virtual void SetFixedCount(int count);
  /// @brief 对图形进行缩放.
  /// factor 正数放大, 负数缩小.
  virtual void Zoom(int factor);

public:
  static std::string ClassName() {
    return "WINEX_CLS_KChart";
  }

protected:
  virtual void OnSetCrosshairPoint(Point point);

  LRESULT CALLBACK OnMessage(
          UINT msg, WPARAM wParam, LPARAM lParam
  );
  virtual LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
  virtual LRESULT OnSize(WPARAM wParam, LPARAM lParam);
  virtual LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
  virtual LRESULT OnLBtnDown(WPARAM wParam, LPARAM lParam);
  virtual LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);

private:
  void FillDrawData(GraphArea *area, DrawData &data);
  void Layout();
  void FitNewWidth();

private:
  HWND handle_;

  Color borderColor_;
  Color backColor_;
  Size size_;

  // 是否显示纵轴坐标(包括左右两边).
  bool lvVisible_;
  bool rvVisible_;
  Scalar vAxisWidth_;
  std::vector<VerticalAxis *> lvAxis_;
  std::vector<VerticalAxis *> rvAxis_;
  std::vector<GraphArea *> areas_;

  bool crosshairEnable_;
  bool crosshairVisible_;
  Color crosshairColor_;
  Point crosshairPoint_;

  // 控制能够显示的数据范围, 这个范围是可以超过 DataSet
  // 中现存的数据.
  int beginIdx_;
  int endIdx_;
  bool fixedCount_;
  float sWidth_;
  DataSet data_;

  WndProcThunk *procThunk_;
  GdiPlusGC *gcContext_;

};
}

#endif // WINCTRL_KLINEWND_H
