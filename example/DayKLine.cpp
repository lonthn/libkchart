//
// Created by luo-zeqi on 2013/4/13.
//

#include "KChartWnd.h"
#include "Indicator.h"

#include <vector>
#include <fstream>

#define CKEY_DATE "DATE"

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

  // 将文件中的历史日K数据加载到 DataSet 中,
  DataSet &data = wnd->DataRef();
  LoadKLineData("SZ000001.csv", data);

  wnd->GetHAxis()->SetScaleCKey(data.FindCol(CKEY_DATE));

  GraphArea *mainArea = wnd->CreateArea(0.6f);
  GraphArea *ind1Area = wnd->CreateArea(0.2f);
  GraphArea *ind2Area = wnd->CreateArea(0.2f);

  // 由于成交量数字较大, 在展示刻度时可以用带单位的形式.
  ind1Area->GetLeftAxis()->SetScaleFormatter(ToStringWithUnit);
  ind1Area->GetRightAxis()->SetScaleFormatter(ToStringWithUnit);

  // 在主图区域添加K线图形, 需要用到数据集中的开高低收4列数据.
  mainArea->AddGraphics(new KLineGraph(data));
  // 添加主图指标MA.
  mainArea->AddGraphics(MA(data, {5, 10, 15, 30, 60}));
  // 副图成交量以及MACD指标.
  ind1Area->AddGraphics(new VolumeGraph(data));
  ind2Area->AddGraphics(MACD(data));

  // MACD 指标图需要中心轴.
  ind2Area->SetCentralAxis(0);

  wnd->Show(TRUE);
  MessageLoop();
  return 0;
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
  ifs.getline(buf, bufSize - 1, '\n');
  std::string line(buf);
  std::vector<std::string> fields;
  StrSplit(line.c_str(), ",", true, fields);
  for (int i = 1; i < fields.size(); ++i) {
    columns.push_back(data.AddCol(fields[i]));
    if (fields[i] == "OPEN"
     || fields[i] == "HIGH"
     || fields[i] == "LOW"
     || fields[i] == "CLOSE") {
      columns.back()->precision = 100;
    }
  }

  while (!ifs.eof()) {
    fields.clear();
    ifs.getline(buf, bufSize - 1, '\n');
    line.assign(buf);

    if (line.empty()) break;
    StrSplit(line.c_str(), ",", true, fields);

    int idx = data.AddRow();
    for (int i = 0; i < columns.size(); ++i) {
      DataType val = std::strtoll(fields[i + 1].c_str(), &endptr, 10);
      data.Set(columns[i], idx, val);
    }
  }
}

void OnKeyDown(WPARAM wParam) {
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

void MessageLoop() {
  MSG msg;
  while (wnd->Handle() && ::GetMessageA(&msg, NULL, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);

    if (msg.message == WM_KEYDOWN) {
      OnKeyDown(msg.wParam);
    }
  }
}