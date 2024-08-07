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

/// @brief ����ͼ��Ĵ�����, ����Ϊ������������, Ҳ������
/// �����е�һ���ؼ�.
class KChartWnd {
public:
  KChartWnd();
  /**
   * @brief ����һ��ͼ�괰�壬��Ҫ�ṩһ�����ݱ�
   * ͼ�����ݽ�ͨ�������ݱ��ȡ�����߿��Ը������
   * ����ʹ���޲ι��� KChartWnd()�����ڲ����д�����
   * @param data ͼ�����ݱ�����Ϊ null.
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

  /// @brief ������õĺ���, ʹ��WindowsAPI��������
  /// hParent ��������ľ��, ��Ϊ��.
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
  /// @brief ʹ��������ʧЧ, �Ӷ������ػ��¼�
  /// ��ʹ���淢���˱仯, ����Ҫ���ô˺��������ػ�.
  void Invalidate();

  /// @brief �л�����: �ռ��ҹ��
  void ChangeTheme(bool white);

  Rect GetAreaBounds();

  /**
   * @brief ����ͼ������, ���Դ������(���½ṹ),
   * ÿ�������Ƕ���������ϵ, percent ����ָ����
   * ��Ȩ��(�ڲ�����Ȩ�ط���߶�).
   */
  GraphArea *CreateArea(float percent);
  /**
   * ���ú���
   * @param axis �ṩһ��������д�ĺ���ʵ��.
   * @return
   */
  void SetHAxis(HorizontalAxis *axis);
  HorizontalAxis *GetHAxis();
  /**
   * @brief ���ý�����չʾ�����������̶�
   * ���ú��޷�ʹ�� Zoom �� MoveIdx
   * @param count �����㼴�̶��������෴��ȡ���̶�
   */
  virtual void SetFixedCount(int count);
  /// @brief ��ͼ�ν�������.
  /// factor �����Ŵ�, ������С.
  virtual void Zoom(int factor);
  /// @brief ���������ƶ�ʮ����.
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

  // ���浱ǰ��������, ͨ���۲����ݼ��еı仯
  // �ж��Ƿ�����������.
  int rowCount_;

  Color borderColor_;
  Color backColor_;
  Size size_;

  // �Ƿ���ʾ��������(������������).
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

  // �����ܹ���ʾ�����ݷ�Χ, �����Χ�ǿ��Գ��� DataSet
  // ���ִ������, ���������ž���ͼ��������ұ߻����
  // һƬ�հ�.
  int  beginIdx_;
  int  endIdx_;
  bool fixedCount_;
  // ����ͼ����ռ���, ��ͨ���ǵ���, ֻ����ͼ�α�������
  // С��3������ʱ,�Ż����˫��, ������ʱӦ�ý������Ϊ
  // ͼ��֮��ľ��������, ��ͼ�εĿ�Ȼᱻ��Ϊ1
  float sWidth_;

  std::shared_ptr<DataSet> data_;

  WndProcThunk *procThunk_;
  GraphContext *gcContext_;

};

}

#endif // WINCTRL_KLINEWND_H
