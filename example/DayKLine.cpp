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

#include "KChartWnd.h"
#include "Indicator.h"

#include <vector>
#include <fstream>

using namespace kchart;

void LoadKLineData(const char *file, DataSet &data);
void Build(DataSet &data);
int  MessageLoop();

static KChartWnd *wnd;

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd
) {
  DataSet data;
  LoadKLineData("../SZ000001.csv", data);

  wnd = new KChartWnd(data);
  wnd->CreateWin(NULL);

  Build(data);

  // 通知计算指标数据
  data.Notify();

  wnd->Show(TRUE);
  return MessageLoop();
}

void Build(DataSet &data) {
  GraphArea *mainArea = wnd->CreateArea(0.6f);
  GraphArea *ind1Area = wnd->CreateArea(0.2f);
  GraphArea *ind2Area = wnd->CreateArea(0.2f);

  wnd->GetHAxis()->SetScaleCKey(data.FindCol("DATE"));

  // 由于成交量数字较大, 在展示刻度时可以用带单位的形式.
  ind1Area->GetLeftAxis()->SetFormatter(ToStrWithUnit);
  ind1Area->GetRightAxis()->SetFormatter(ToStrWithUnit);
  ind1Area->SetZeroOrigin(true);
  // MACD 指标图需要中心轴.
  ind2Area->SetCentralAxis(0);

  // 在主图区域添加K线图形, 需要用到数据集中的开高低收4列数据.
  mainArea->AddGraphics(new KLineGraph(data));
  // 添加主图指标MA.
  mainArea->AddGraphics(MA(data, {5, 10, 15, 30, 60}));
  // 副图成交量以及MACD指标.
  ind1Area->AddGraphics(new VolumeGraph(data));
  ind2Area->AddGraphics(MACD(data));
}


void LoadKLineData(const char *file, DataSet &data) {
//  symbol,date,open,high,low,close,volume
//  SH000001,20220104,7688720,8037480,7674470,8037480,302111788,2928518397
//  SH000001,20220105,7688720,8037480,7674470,8037480,302111788,2928518397
//  ...

  std::ifstream ifs(file, std::ios::binary);
  assert(ifs.is_open());

  std::vector<ColumnKey> columns;

  const int bufSize = 1024;
  char buf[bufSize];
  char *endptr = nullptr;

  // Skip the header.
  ifs.getline(buf, bufSize);
  std::vector<std::string> fields;
  StrSplit(buf, ",", true, fields);
  for (int i = 1; i < fields.size(); ++i)
    columns.push_back(data.CreateCol(fields[i]));

  while (!ifs.eof()) {
    ifs.getline(buf, bufSize - 1, '\n');
    if (strlen(buf) == 0) break;

    const char *str = strstr(buf, ",") + 1;
    int idx = data.AddRow();
    for (auto & column : columns) {
      DataType val = std::strtoll(str, &endptr, 10);
      data.Set(column, idx, val);
      str = endptr + 1;
    }
  }
}

void OnKeyDown(WPARAM wParam) {
  // 组合键判断
  if (GetKeyState(VK_SHIFT) < 0) {
    if (wParam == VK_LEFT) {
      wnd->FastScroll(-1);
      wnd->Invalidate();
    } else if (wParam == VK_RIGHT) {
      wnd->FastScroll(1);
      wnd->Invalidate();
    }
    return;
  }

  if (wParam == VK_UP) {
    wnd->Zoom(1);
    wnd->Invalidate();
  } else if (wParam == VK_DOWN) {
    wnd->Zoom(-1);
    wnd->Invalidate();
  } else if (wParam == VK_F1) {
    wnd->ChangeTheme(true);
    wnd->Invalidate();
  } else if (wParam == VK_F2) {
    wnd->ChangeTheme(false);
    wnd->Invalidate();
  } else if (wParam == VK_LEFT) {
    wnd->MoveCrosshair(-1);
    wnd->Invalidate();
  } else if (wParam == VK_RIGHT) {
    wnd->MoveCrosshair(1);
    wnd->Invalidate();
  }
}

int MessageLoop() {
  // StartPull(wnd->Handle());

  MSG msg;
  while (wnd->Handle() && ::GetMessageA(&msg, NULL, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);

    if (msg.message == WM_KEYDOWN) {
      OnKeyDown(msg.wParam);
    }
  }

  // StopPull();
  return 0;
}
