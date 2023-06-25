//
// Created by luo-zeqi on 2013/4/17.
//

#ifndef WINCTRL_SHAPE_H
#define WINCTRL_SHAPE_H

#include "Def.h"
#include "Struct.h"
#include "graph/GraphContext.h"
#include "Graphics.h"

#include <atlstr.h>

namespace kchart {

class KChartWnd;
class VerticalAxis;

/**
 * ͼ�����������²��ֵķ�ʽչʾ�� KCharWnd ��.
 */
class GraphArea {
public:
  virtual ~GraphArea();

  virtual VerticalAxis *GetLeftAxis() {
    return lAxis_;
  }
  virtual VerticalAxis *GetRightAxis() {
    return rAxis_;
  }

  DataType GetCentralAxis();
  /**
   * ����һ��������
   * @param n NAN ��ʾȡ������.
   */
  void SetCentralAxis(DataType n);
  /**
   * �Ƿ���Ҫǿ���������չʾ.
   * @param flag
   */
  void SetBoldCentralAxis(bool flag);
  void SetDecimals(int decimals);
  void SetLabelVisible(bool flag);
  void SetLabelBackColor(Color color);
  void SetScaleLineColor(Color color);

  bool AddGraphics(Graphics *graph);
  bool AddGraphics(const std::vector<Graphics *> &graph);

protected:
  explicit GraphArea(KChartWnd *panel,
                     VerticalAxis *la,
                     VerticalAxis *ra);

  void SetWeight(float weight);
  float GetWeight() const;
  void SetBounds(const Rect &bounds);
  Rect GetBounds() const;
  Scalar GetLabelHeight() const;
  // ����ǰ����չʾ���������ݵ� min max
  DataType GetMin() const;
  DataType GetMax() const;
  const DataRows &GetScales();
  Point GetCrosshairPoint() const;
  int GetCrosshairIndex() const;

  Scalar GetContentTop() const;
  Scalar GetContentHeight() const;
  void ReGatherLabel(GraphContext *ctx, DrawData &data);

  virtual void UpdateMinMax();
  virtual void UpdateScales();

  virtual void OnFitIdx(int begin, int end);
  virtual void OnMoveCrosshair(Point point);
  virtual void OnPaint(GraphContext *ctx, DrawData &data);
  virtual void OnPaintLabel(GraphContext *ctx, DrawData &data);
  virtual void OnPaintGraph(GraphContext *ctx, DrawData &data);
  virtual void OnPaintCrosshair(GraphContext *ctx, DrawData &data);

protected:
  KChartWnd *panel_;
  float weight_;

  Rect bounds_;
  Rect graphArea_;
  Point crosshairPoint_;
  int crosshairIndex_;
  Scalar labelHeight_;
  bool labelVisible_;
  Color labelBackColor_;

  /// ʮ��������λ�����ݿ���ͨ����ǩչʾ.
  struct Label {
    CStringW text;
    Color color;
    Size size;
  };
  std::vector<Label> labels_;

  int begin_;
  int end_;
  DataType cacheMin_;
  DataType cacheMax_;
  bool validCache_;
  int decimals_;

  Color scaleLineColor_;
  int colorIdx_;
  // ������ɫ�б�, ��ͼ��ʹ��.
  std::vector<Color> colorList_;
  DataRows scales_;
  VerticalAxis *lAxis_;
  VerticalAxis *rAxis_;
  DataType centralAxis_;
  bool boldCA_;

  std::vector<Graphics *> graphics_;

  friend KChartWnd;
  friend VerticalAxis;
};

}

#endif //WINCTRL_SHAPE_H
