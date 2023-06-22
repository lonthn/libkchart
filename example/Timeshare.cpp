//
// Created by luo-zeqi on 2023/5/24.
//

#include "KChartWnd.h"
#include "Indicator.h"

#include <vector>
#include <fstream>

#define CKEY_OPEN   "OPEN"
#define CKEY_CLOSE  "CLOSE"
#define CKEY_AVG    "AVG-PRICE"
#define CKEY_VOLUME "VOLUME"

using namespace kchart;

void LoadTimeshareData(const char *file, DataSet &data);
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

  // 241条分时数据
  wnd->SetFixedCount(241);

  GraphArea *mainArea = wnd->CreateArea(0.7f);
  GraphArea *indiArea = wnd->CreateArea(0.3f);

  // 由于成交量数字较大, 在展示刻度时可以用带单位的形式.
  indiArea->GetLeftAxis()->SetScaleFormatter(ToStringWithUnit);
  indiArea->GetRightAxis()->SetScaleFormatter(ToStringWithUnit);

  // 将文件中的历史数据加载到 DataSet 中
  DataSet &data = wnd->DataRef();
  LoadTimeshareData("SH000001-min.csv", data);

  // 将中心轴设置为第一条数据.
  ColumnKey colKey = data.FindCol(CKEY_CLOSE);
  mainArea->SetCentralAxis(data.Get(colKey, 0));
  mainArea->SetBoldCentralAxis(true);

  // 在主图区域添加最新价以及均价线.
  mainArea->AddGraphics(new PolyLineGraph(data.FindCol(CKEY_CLOSE)));
  mainArea->AddGraphics(new PolyLineGraph(data.FindCol(CKEY_AVG)));
  // 副图成交量.
  indiArea->AddGraphics(new VolumeGraph(data));
  indiArea->SetLabelVisible(false);

  wnd->Show(TRUE);
  MessageLoop();
  return 0;
}

void LoadTimeshareData(const char *file, DataSet &data) {
  // 注意! 文件中的数据并不是真实市场行情数据, 不具备任何参考意义.
  std::ifstream ifs(file, std::ios::binary);
  assert(ifs.is_open());

  ColumnKey close = data.AddCol(CKEY_CLOSE);
  ColumnKey vol = data.AddCol(CKEY_VOLUME);
  ColumnKey avg = data.AddCol(CKEY_AVG);
  ColumnKey open = data.AddCol(CKEY_OPEN);

  const int bufSize = 1024;
  char buf[bufSize];

  // Skip the header.
  ifs.getline(buf, bufSize - 1, '\n');

  char *endptr = nullptr;
  DataType preVol = 0;
  while (!ifs.eof()) {
    ifs.getline(buf, bufSize - 1, '\n');

    CStringA line(buf);
    std::vector<CStringA> fields;
    if (line.IsEmpty()) continue;
    StrSplit(line.GetString(), ",", true, fields);

    int idx = data.AddRow();
    DataType volume = std::strtof(fields[2], &endptr);
    data.Set(close, idx, std::strtof(fields[1], &endptr) / 3);
    data.Set(vol, idx, volume - preVol);
    data.Set(avg, idx, std::strtof(fields[3], &endptr) / 3);
    data.Set(open, idx, std::strtof(fields[3], &endptr) / 3);
    preVol = volume;
  }
}

void MessageLoop() {
  MSG msg;
  while (wnd->Handle() && ::GetMessageA(&msg, NULL, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }
}