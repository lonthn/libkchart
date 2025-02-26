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

#include "../kchart.h"
#include "Indicator.h"

#include <vector>
#include <fstream>
#include <cassert>

#define CKEY_OPEN   "OPEN"
#define CKEY_CLOSE  "CLOSE"
#define CKEY_AVG    "AVG-PRICE"
#define CKEY_ORDERR "ORD-RATIO"
#define CKEY_VOLUME "VOLUME"
#define CKEY_TIME   "TIME"

using namespace kchart;

void LoadTimeshareData(const char *file, DataSet &data);
void Build();
int  MessageLoop();

KChartWnd *wnd = NULL;

int main() {
  auto data = std::make_shared<DataSet>();

  // ���ļ��е���ʷ���ݼ��ص� DataSet ��
  LoadTimeshareData("SH000001-min.csv", *data);

  wnd = new KChartWnd(data);
  wnd->CreateWin(NULL);

  Build();

  wnd->Show(TRUE);
  return MessageLoop();
}

void Build() {
  DataSet &data = wnd->DataRef();

  // 241����ʱ����
  wnd->SetFixedCount(241);
  wnd->GetHAxis()->SetScaleCKey(data.FindCol(CKEY_TIME));

  GraphArea *mainArea = wnd->CreateArea(0.7f);
  GraphArea *indiArea = wnd->CreateArea(0.3f);

  // ���ڳɽ������ֽϴ�, ��չʾ�̶�ʱ�����ô���λ����ʽ.
  indiArea->GetLeftAxis()->SetFormatter(ToStrWithUnit);
  indiArea->GetRightAxis()->SetFormatter(ToStrWithUnit);

  // ������������Ϊ��һ������.
  ColumnKey colKey = data.FindCol(CKEY_CLOSE);
  mainArea->SetCentralAxis(data.Get(colKey, 0));
  mainArea->SetBoldCentralAxis(true);

  // ����ͼ����������¼��Լ�������.
  mainArea->AddGraphics(new HistogramGraph(data.FindCol(CKEY_ORDERR), 1));
  mainArea->AddGraphics(new PolyLineGraph(data.FindCol(CKEY_CLOSE)));
  mainArea->AddGraphics(new PolyLineGraph(data.FindCol(CKEY_AVG)));

  // ��ͼ�ɽ���.
  indiArea->AddGraphics(new VolumeGraph(data));
  indiArea->SetZeroOrigin(true);
  //indiArea->SetLabelVisible(false);

  data.Notify();
}

void LoadTimeshareData(const char *file, DataSet &data) {
  // ע��! ������ʵ�г���������, ���߱��κβο�����.
  std::ifstream ifs(file, std::ios::binary);
  assert(ifs.is_open());

  ColumnKey close = data.CreateCol(CKEY_CLOSE, 10000);
  ColumnKey vol = data.CreateCol(CKEY_VOLUME);
  ColumnKey avg = data.CreateCol(CKEY_AVG, 10000);
  ColumnKey open = data.CreateCol(CKEY_OPEN, 10000);
  ColumnKey ord = data.CreateCol(CKEY_ORDERR, 10000);
  ColumnKey time = data.CreateCol(CKEY_TIME);

  const int bufSize = 1024;
  char buf[bufSize];

  // Skip the header.
  ifs.getline(buf, bufSize - 1, '\n');

  DataType minv = INT64_MAX - 1;
  DataType maxv = INT64_MIN + 1;

  char *endptr = nullptr;
  DataType preVol = 0;
  while (!ifs.eof()) {
    ifs.getline(buf, bufSize - 1, '\n');

    std::string line(buf);
    std::vector<std::string> fields;
    if (line.empty()) continue;
    StrSplit(line.c_str(), ",", true, fields);

    int idx = data.AddRow();
    DataType volume = std::strtoll(fields[2].c_str(), &endptr, 10);
    data.Set(time,  idx, std::strtoll(fields[0].c_str(), &endptr, 10));
    data.Set(close, idx, std::strtoll(fields[1].c_str(), &endptr, 10));
    data.Set(vol,   idx, volume);
    data.Set(avg,   idx, std::strtoll(fields[3].c_str(), &endptr, 10));
    // ���̼۲���չʾ������ֻ��Ϊ��ȷ���ɽ�����ͼ����ɫ.
    if (rand() % 2 == 1) {
      data.Set(open, idx, data.Get(close, idx) + 1);
    } else {
      data.Set(open, idx, data.Get(close, idx) - 1);
    }
    minv = min(minv, data.Get(close, idx));
    maxv = max(maxv, data.Get(close, idx));
  }

  // �������ÿ���ӵ������̱�.
  DataType x = maxv - minv;
  for (int i = 0; i < data.RowCount(); ++i) {
    int   sign = rand() % 2;
    int   percent = rand() % 100;
    float rate = (float)percent / 100 * 0.15f;
    if (sign == 0)
      rate *= -1;
    data.Set(ord, i, data.Get(close, 0) + DataType((float)x * rate));
  }
}

int MessageLoop() {
  MSG msg;
  while (wnd->Handle() && ::GetMessageA(&msg, NULL, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);

    if (msg.message == WM_KEYDOWN) {
      if (msg.wParam == VK_F1) {
        wnd->ChangeTheme(true);
        wnd->Invalidate();
      } else if (msg.wParam == VK_F2) {
        wnd->ChangeTheme(false);
        wnd->Invalidate();
      }
    }
  }

  return 0;
}