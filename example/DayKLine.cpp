//
// Created by luo-zeqi on 2013/4/13.
//

#include "KChartWnd.h"
#include "Indicator.h"

#include <vector>
#include <fstream>

using namespace kchart;

void LoadKLineData(const char *file, DataSet &data);
void MessageLoop();

KChartWnd *wnd = NULL;

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd
) {
  wnd = new KChartWnd();
  wnd->CreateWin(NULL);

  GraphArea *mainArea = wnd->CreateArea(0.6f);
  GraphArea *ind1Area = wnd->CreateArea(0.2f);
  GraphArea *ind2Area = wnd->CreateArea(0.2f);

  // 由于成交量数字较大, 在展示刻度时可以用带单位的形式.
  ind1Area->GetLeftAxis()->SetScaleFormatter(ToStringWithUnit);
  ind1Area->GetRightAxis()->SetScaleFormatter(ToStringWithUnit);

  // 将文件中的历史日K数据加载到 DataSet 中,
  DataSet &data = wnd->DataRef();
  LoadKLineData("SZ000001.csv", data);

  // 在主图区域添加K线图形, 需要用到数据集中的开高低收4列数据.
  mainArea->AddGraphics(new KLineGraph(data));
  // 添加主图指标MA.
  mainArea->AddGraphics(MA(data, {5, 10, 15, 30, 60}));
  // 副图成交量以及MACD指标.
  ind1Area->AddGraphics(new VolumeGraph(data));
  ind2Area->AddGraphics(MACD(data));

  // MACD 指标图需要中心轴.
  ind2Area->SetCentralAxis(0.0f);

  wnd->Show(TRUE);
  MessageLoop();
  return 0;
}

void LoadKLineData(const char *file, DataSet &data) {
//  symbol,date,open,high,low,close,volume
//  SH000001,20220104,768.8720,803.7480,767.4470,803.7480,302111788,2928518397
//  SH000001,20220105,768.8720,803.7480,767.4470,803.7480,302111788,2928518397
//  ...

  std::ifstream ifs(file, std::ios::binary);
  assert(ifs.is_open());

  ColumnKey open = data.AddCol("OPEN");
  ColumnKey high = data.AddCol("HIGH");
  ColumnKey low = data.AddCol("LOW");
  ColumnKey close = data.AddCol("CLOSE");
  ColumnKey vol = data.AddCol("VOLUME");

  const int bufSize = 1024;
  char buf[bufSize];

  // Skip the header.
  ifs.getline(buf, bufSize - 1, '\n');

  while (!ifs.eof()) {
    ifs.getline(buf, bufSize - 1, '\n');

    CStringA line(buf);
    if (line.IsEmpty())
      continue;

    std::vector<CStringA> fields;
    StrSplit(line.GetString(), ",", true, fields);

    char *endptr = nullptr;

    int idx = data.AddRow();
    data.Set(open, idx, std::strtof(fields[2], &endptr));
    data.Set(high, idx, std::strtof(fields[3], &endptr));
    data.Set(low, idx, std::strtof(fields[4], &endptr));
    data.Set(close, idx, std::strtof(fields[5], &endptr));
    data.Set(vol, idx, std::strtof(fields[6], &endptr));
  }
}

void MessageLoop() {
  MSG msg;
  while (wnd->Handle() && ::GetMessageA(&msg, NULL, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);

    if (msg.message == WM_KEYDOWN) {
      if (msg.wParam == VK_UP) {
        wnd->Zoom(1);
        wnd->Invalidate();
      } else if (msg.wParam == VK_DOWN) {
        wnd->Zoom(-1);
        wnd->Invalidate();
      }
    }
  }
}