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
    fontFamilies_[FontId_WRYH]  = L"΢���ź�";
  }

public:
  // ����ԭ��λ��.
  virtual Point Translate(const Point &point) { return {0, 0}; }
  virtual void SetTranslate(const Point &point) {}

  virtual void SetFont(FontId fontId, int size) {}
  virtual void SetColor(Color color) {}
  virtual void SetLineWidth(int width) {}

  /**
   * ��Ⱦ����, DrawRect ������ĵ�, FillRect ����ʵ��.
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
   * �ṩ������, ��һ���߶�.
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
   * ��һ������������, ��2�������ϵĵ����.
   * @param pointNum Ԥ��ָ���۵�����.
   * @example ʹ��ʱ��Ҫ�õ�3���������ڵ���EndPolyLineʱ
   * �����е�������Ⱦ�ڻ�����:
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
   * ���ַ��� str ��Ⱦ�ڻ�����
   * @param str
   * @param p �ı����Ͻǵ�λ��.
   */
  virtual void DrawStr(const CStringW &str, const Point &p) {}
  // Ҫ�����ı�, ͨ��������Ҫ �����ı��Ŀ�ߣ�
  // ���ǻῼ�� SetFont ������ size ����.
  virtual Size MeasureStr(const CStringW &str) { return {0, 0}; }
};

}

#endif //LIBKCHART_GRAPHCONTEXT_H
