//
// Created by luo-zeqi on 2023/6/22.
//

#ifndef LIBKCHART_INDICATOR_H
#define LIBKCHART_INDICATOR_H

#include "KChartWnd.h"

#include <vector>

using namespace kchart;

/* ����ָ�����ݲ���ͼ��. ע�⣺����ָ��������� CLOSE ������. */

std::vector<Graphics *> MA(DataSet &data, const std::vector<int> &nums) {
  int row = data.RowCount();
  ColumnKey close = data.FindCol("CLOSE");

  auto ma = [&](int n) {
    ColumnKey ma = data.AddCol("MA" + std::to_string(n));

    DataType closeSum = 0;
    int i = 0;
    int off = n - 1;
    for (; i < off; i++) {
      closeSum += data.Get(close, i);
      data.Set(ma, i, NAN); // NAN ��ʾ������, ���û���.
    }

    for (; i < row; i++) {
      closeSum += data.Get(close, i);
      // ����MA(n) = ǰn-1���Լ��������̼�֮�� / n
      data.Set(ma, i, closeSum / DataType(n));
      closeSum -= data.Get(close, i - off);
    }

    return new PolyLineGraph(ma);
  };

  std::vector<Graphics *> graph;
  for (int n: nums)
    graph.push_back(ma(n));

  return graph;
}

std::vector<Graphics *> MACD(DataSet &data) {
  ColumnKey close = data.FindCol("CLOSE");

  ColumnKey dif = data.AddCol("DIF");
  ColumnKey dea = data.AddCol("DEA");
  ColumnKey macd = data.AddCol("MACD");

  DataType n1 = 12;
  DataType sc11 = (n1 - 1) / (n1 + 1);
  DataType sc12 = 2 / (n1 + 1);
  DataType n2 = 26;
  DataType sc21 = (n2 - 1) / (n2 + 1);
  DataType sc22 = (2) / (n2 + 1);
  DataType n3 = 9;
  DataType sc31 = (n3 - 1) / (n3 + 1);
  DataType sc32 = (2) / (n3 + 1);

  DataType preEMA1 = 0, preEMA2 = 0, preDEA = 0;
  for (int i = 0; i < data.RowCount(); i++) {
    // EMA(n1) = ǰһ��EMA(n1)��(n1-1)/(n1+1) + �������̼ۡ�2/(n1+1)
    preEMA1 = preEMA1 * sc11 + data.Get(close, i) * sc12;
    // EMA(n2) = ǰһ��EMA(n2)��(n2-1)/(n2+1) + �������̼ۡ�2/(n2+1)
    preEMA2 = preEMA2 * sc21 + data.Get(close, i) * sc22;
    // DIF = ����EMA(n1) �� ����EMA(n2)
    DataType difv = preEMA1 - preEMA2;
    // ����DEA = ǰһ��DEA��(n3-1)/(n3+1) + ����DIF��2/(n3+1)
    preDEA = preDEA * sc31 + difv * sc32;

    data.Set(dif, i, difv);
    data.Set(dea, i, preDEA);
    data.Set(macd, i, (difv - preDEA) * 2);
  }

  std::vector<Graphics *> graphics;

  HistogramGraph *hg = new HistogramGraph(macd);
  hg->FixedWidth = 1; // macd ʹ�õ�������ͼ

  graphics.push_back(hg);
  graphics.push_back(new PolyLineGraph(dif));
  graphics.push_back(new PolyLineGraph(dea));

  return graphics;
}

#endif //LIBKCHART_INDICATOR_H
