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

#ifndef LIBKCHART_INDICATOR_H
#define LIBKCHART_INDICATOR_H

#include "KChartWnd.h"

#include <vector>

using namespace kchart;

/* 计算指标数据并绑定图形. 注意：以下指标计算依赖 CLOSE 列数据. */

std::vector<Graphics *> MA(DataSet &data, const std::vector<int> &nums) {
  auto calc = [](DataSet &data, int oldRow, int maNum) {
    int newRow = data.RowCount();
    ColumnKey close = data.FindCol("CLOSE");
    ColumnKey ma = data.FindCol("MA" + std::to_string(maNum));

    if (newRow < maNum)
      return;

    int i = oldRow - maNum;
    if (i < 0) i = 0;

    DataType closeSum = 0;
    int n = i + maNum - 1;
    for (; i < n; i++) {
      closeSum += data.Get(close, i);
      // KC_INVALID_DATA 表示空数据, 不用绘制.
      if (i < maNum)
        data.Set(ma, i, KC_INVALID_DATA);
    }

    for (; i < newRow; i++) {
      closeSum += data.Get(close, i);
      // 当日MA(n) = 前n-1日以及当日收盘价之和 / n
      data.Set(ma, i, closeSum / DataType(maNum));
      closeSum -= data.Get(close, i - (maNum - 1));
    }
  };

  std::vector<Graphics *> graph;
  graph.reserve(nums.size());
  for (int n : nums) {
    ColumnKey ma = data.CreateCol("MA" + std::to_string(n), 100);
    graph.push_back(new PolyLineGraph(ma));
    data.AddObserver(0, [&, n](DataSet &data, int oldRow) {
      calc(data, oldRow, n);
    });
  }
  return graph;
}

std::vector<Graphics *> MACD(DataSet &data) {
  ColumnKey dif = data.CreateCol("DIF", 100);
  ColumnKey dea = data.CreateCol("DEA", 100);
  ColumnKey macd = data.CreateCol("MACD", 100);

  auto calc = [=](DataSet &data, int oldRow) {
    ColumnKey close = data.FindCol("CLOSE");

    float n1 = 12, n2 = 26, n3 = 9;
    float sc11 = float(n1 - 1) / float(n1 + 1);
    float sc12 = 2 / float(n1 + 1);
    float sc21 = float(n2 - 1) / float(n2 + 1);
    float sc22 = 2 / float(n2 + 1);
    float sc31 = float(n3 - 1) / float(n3 + 1);
    float sc32 = 2 / float(n3 + 1);

    //static float preEMA1 = 0, preEMA2 = 0, preDEA = 0;
    static float EMA1 = 0, EMA2 = 0, DEA = 0;
    if (data.Empty())
      return;

    int i = max(0, oldRow - 1);
  loop:
    // EMA(n1) = 前一日EMA(n1)×(n1-1)/(n1+1) + 今日收盘价×2/(n1+1)
    float tEMA1 = EMA1 * sc11 + float(data.Get(close, i)) * sc12;
    // EMA(n2) = 前一日EMA(n2)×(n2-1)/(n2+1) + 今日收盘价×2/(n2+1)
    float tEMA2 = EMA2 * sc21 + float(data.Get(close, i)) * sc22;
    // DIF = 今日EMA(n1) － 今日EMA(n2)
    float difv = tEMA1 - tEMA2;
    // 今日DEA = 前一日DEA×(n3-1)/(n3+1) + 今日DIF×2/(n3+1)
    float tDEA = DEA * sc31 + difv * sc32;

    data.Set(dif, i, DataType(difv));
    data.Set(dea, i, DataType(DEA));
    data.Set(macd, i, DataType((difv - DEA) * 2));
    if ((++i) < data.RowCount()) {
      EMA1 = tEMA1; EMA2 = tEMA2; DEA = tDEA;
      goto loop;
    }

//    for (int i = oldRow; i < data.RowCount(); i++) {
//      // EMA(n1) = 前一日EMA(n1)×(n1-1)/(n1+1) + 今日收盘价×2/(n1+1)
//      EMA1 = EMA1 * sc11 + float(data.Get(close, i)) * sc12;
//      // EMA(n2) = 前一日EMA(n2)×(n2-1)/(n2+1) + 今日收盘价×2/(n2+1)
//      EMA2 = EMA2 * sc21 + float(data.Get(close, i)) * sc22;
//      // DIF = 今日EMA(n1) － 今日EMA(n2)
//      float difv = EMA1 - EMA2;
//      // 今日DEA = 前一日DEA×(n3-1)/(n3+1) + 今日DIF×2/(n3+1)
//      DEA = DEA * sc31 + difv * sc32;
//
//      data.Set(dif, i, DataType(difv));
//      data.Set(dea, i, DataType(DEA));
//      data.Set(macd, i, DataType((difv - DEA) * 2));
//    }
  };

  data.AddObserver(0, calc);

  std::vector<Graphics *> graphics;
  graphics.push_back(new HistogramGraph(macd, 1));
  graphics.push_back(new PolyLineGraph(dif));
  graphics.push_back(new PolyLineGraph(dea));

  return graphics;
}

#endif //LIBKCHART_INDICATOR_H
