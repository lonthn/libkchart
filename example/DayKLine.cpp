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

  // ���ļ��е���ʷ��K���ݼ��ص� DataSet ��,
  DataSet &data = wnd->DataRef();
  LoadKLineData("SZ000001.csv", data);

  wnd->GetHAxis()->SetScaleCKey(data.FindCol(CKEY_DATE));

  GraphArea *mainArea = wnd->CreateArea(0.6f);
  GraphArea *ind1Area = wnd->CreateArea(0.2f);
  GraphArea *ind2Area = wnd->CreateArea(0.2f);

  // ���ڳɽ������ֽϴ�, ��չʾ�̶�ʱ�����ô���λ����ʽ.
  ind1Area->GetLeftAxis()->SetScaleFormatter(ToStringWithUnit);
  ind1Area->GetRightAxis()->SetScaleFormatter(ToStringWithUnit);

  // ����ͼ�������K��ͼ��, ��Ҫ�õ����ݼ��еĿ��ߵ���4������.
  mainArea->AddGraphics(new KLineGraph(data));
  // �����ͼָ��MA.
  mainArea->AddGraphics(MA(data, {5, 10, 15, 30, 60}));
  // ��ͼ�ɽ����Լ�MACDָ��.
  ind1Area->AddGraphics(new VolumeGraph(data));
  ind2Area->AddGraphics(MACD(data));

  // MACD ָ��ͼ��Ҫ������.
  ind2Area->SetCentralAxis(0.0f);

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
      } else if (msg.wParam == VK_F1) {
        wnd->ChangeTheme(true);
        wnd->Invalidate();
      } else if (msg.wParam == VK_F2) {
        wnd->ChangeTheme(false);
        wnd->Invalidate();
      }
    }
  }
}