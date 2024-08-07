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

#ifndef WINCTRL_KLINEWND_H
#define WINCTRL_KLINEWND_H

#include "GraphArea.h"
#include "Graphics.h"
#include "Struct.h"
#include "VerticalAxis.h"
#include "HorizontalAxis.h"

#include <Windows.h>

class WndProcThunk;

namespace kchart {

/// @brief 容纳图表的窗体类, 可作为独立窗口运行, 也可以是
/// 窗体中的一个控件.
class KChartWnd {
public:
  KChartWnd();
  /**
   * @brief 构造一个图标窗体，需要提供一个数据表
   * 图形数据将通过该数据表获取，或者可以根据情况
   * 考虑使用无参构造 KChartWnd()，由内部自行创建。
   * @param data 图形数据表，不能为 null.
   */
  explicit KChartWnd(std::shared_ptr<DataSet> data);
  virtual ~KChartWnd();

  HWND Handle() {
      return handle_;
  }
  DataSet &DataRef() {
      return *data_;
  }
  void SetCrosshairColor(Color color) {
      crosshairColor_ = color;
  }
  Color GetCrosshairColor() const {
      return crosshairColor_;
  }
  GraphContext *GetGraphContext() {
    return gcContext_;
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

  /// @brief 切换主题: 日间或夜间
  void ChangeTheme(bool white);

  Rect GetAreaBounds();

  /**
   * @brief 创建图形区域, 可以创建多个(上下结构),
   * 每个区域都是独立的坐标系, percent 可以指定高
   * 度权重(内部根据权重分配高度).
   */
  GraphArea *CreateArea(float percent);
  /**
   * 设置横轴
   * @param axis 提供一个经过重写的横轴实例.
   * @return
   */
  void SetHAxis(HorizontalAxis *axis);
  HorizontalAxis *GetHAxis();
  /**
   * @brief 设置将界面展示的数据条数固定
   * 设置后将无法使用 Zoom 和 MoveIdx
   * @param count 大于零即固定条数，相反则取消固定
   */
  virtual void SetFixedCount(int count);
  /// @brief 对图形进行缩放.
  /// factor 正数放大, 负数缩小.
  virtual void Zoom(int factor);
  /// @brief 横向左右移动十字线.
  virtual void MoveCrosshair(int offset);
  virtual void FastScroll(int dir);

public:
  static std::string ClassName() {
    return "WINEX_CLS_KChart";
  }

protected:
  virtual void OnSetCrosshairPoint(Point point);

  LRESULT CALLBACK OnMessage(
          UINT msg, WPARAM wParam, LPARAM lParam
  );

  virtual LRESULT OnProcCreate();
  virtual LRESULT OnProcSize(Scalar width, Scalar height);
  virtual LRESULT OnProcPaint(Rect rect);
  virtual LRESULT OnProcLBtnDown(Point point);
  virtual LRESULT OnProcMouseMove(Point point);

private:
  void FillDrawData(GraphArea *area, DrawData &data);
  void Layout();
  void FitNewWidth();

private:
  HWND handle_;

  // 缓存当前数据条数, 通过观察数据集中的变化
  // 判断是否有新增数据.
  int rowCount_;

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
  HorizontalAxis *hAxis_;

  bool crosshairEnable_;
  bool crosshairVisible_;
  Color crosshairColor_;
  Point crosshairPoint_;

  // 控制能够显示的数据范围, 这个范围是可以超过 DataSet
  // 中现存的数据, 这种情况大概就是图形区域的右边会出现
  // 一片空白.
  int  beginIdx_;
  int  endIdx_;
  bool fixedCount_;
  // 单个图形所占宽度, 它通常是单数, 只有在图形被缩放至
  // 小于3个像素时,才会出现双数, 但在这时应该将其理解为
  // 图形之间的距离更合适, 而图形的宽度会被置为1
  float sWidth_;

  std::shared_ptr<DataSet> data_;

  WndProcThunk *procThunk_;
  GraphContext *gcContext_;

};

}

#endif // WINCTRL_KLINEWND_H
