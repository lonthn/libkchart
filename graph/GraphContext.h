//
// Created by luo-zeqi on 2013/4/17.
//

#ifndef LIBKCHART_GRAPHCONTEXT_H
#define LIBKCHART_GRAPHCONTEXT_H

#include "Scalar.h"
#include "Color.h"

#include <vector>
#include <atlstr.h>

namespace kchart {

typedef int FontId;
static FontId FontId_Arial = 0;
static FontId FontId_WRYH  = 1;

class GraphContext {
public:
  std::vector<CStringW> fontFamilies_;

  GraphContext() {
    fontFamilies_.resize(2);
    fontFamilies_[FontId_Arial] = L"Arial";
    fontFamilies_[FontId_WRYH]  = L"微软雅黑";
  }

public:
  // 控制原点位置.
  virtual Point Translate(const Point &point) { return {0, 0}; }
  virtual void SetTranslate(const Point &point) {}

  virtual void SetFont(FontId fontId, int size) {}
  virtual void SetColor(Color color) {}
  virtual void SetLineWidth(int width) {}

  /**
   * 渲染矩形, DrawRect 代表空心的, FillRect 代表实心.
   * @param r
   */
  virtual void DrawRect(const Rect &r) {}
  virtual void DrawRect(
      Scalar l, Scalar t, Scalar r, Scalar b
  ) {}
  virtual void FillRect(const Rect &r) {}
  virtual void FillRect(
      Scalar l, Scalar t, Scalar r, Scalar b
  ) {}

  /**
   * 提供两个点, 画一条线段.
   * @param a
   * @param b
   */
  virtual void DrawLine(
      const Point &a, const Point &b
  ) {}
  virtual void DrawLine(
      Scalar ax, Scalar ay, Scalar bx, Scalar by
  ) {}

  /**
   * 画一条不规则折线, 由2个或以上的点组成.
   * @param pointNum 预先指定折点数量.
   * @example 使用时需要用到3个函数，在调用EndPolyLine时
   * 将所有点连接渲染在画布上:
   *  ctx.BeginPolyLine(5);
   *  for (int i = 0; i < 5; i++) {
   *    ctx.AddPolyLine({x, y});
   *  }
   *  ctx.EndPolyLine();
   */
  virtual void BeginPolyLine(int pointNum) {}
  virtual void AddPolyLine(const Point &p) {}
  virtual void EndPolyLine() {}

  /**
   * 将字符串 str 渲染在画布上
   * @param str
   * @param p 文本左上角的位置.
   */
  virtual void DrawStr(const CStringW &str, const Point &p) {}
  // 要绘制文本, 通畅可能需要 测量文本的宽高，
  // 我们会考虑 SetFont 函数的 size 参数.
  virtual Size MeasureStr(const CStringW &str) { return {0, 0}; }
};

}

#endif //LIBKCHART_GRAPHCONTEXT_H
