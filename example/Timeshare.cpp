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

  // 241����ʱ����
  wnd->SetFixedCount(241);

  GraphArea *mainArea = wnd->CreateArea(0.7f);
  GraphArea *indiArea = wnd->CreateArea(0.3f);

  // ���ڳɽ������ֽϴ�, ��չʾ�̶�ʱ�����ô���λ����ʽ.
  indiArea->GetLeftAxis()->SetScaleFormatter(ToStringWithUnit);
  indiArea->GetRightAxis()->SetScaleFormatter(ToStringWithUnit);

  // ���ļ��е���ʷ���ݼ��ص� DataSet ��
  DataSet &data = wnd->DataRef();
  LoadTimeshareData("SH000001-min.csv", data);

  // ������������Ϊ��һ������.
  ColumnKey colKey = data.FindCol(CKEY_CLOSE);
  mainArea->SetCentralAxis(data.Get(colKey, 0));
  mainArea->SetBoldCentralAxis(true);

  // ����ͼ����������¼��Լ�������.
  mainArea->AddGraphics(new PolyLineGraph(data.FindCol(CKEY_CLOSE)));
  mainArea->AddGraphics(new PolyLineGraph(data.FindCol(CKEY_AVG)));
  // ��ͼ�ɽ���.
  indiArea->AddGraphics(new VolumeGraph(data));
  indiArea->SetLabelVisible(false);

  wnd->Show(TRUE);
  MessageLoop();
  return 0;
}

void LoadTimeshareData(const char *file, DataSet &data) {
  // ע��! �ļ��е����ݲ�������ʵ�г���������, ���߱��κβο�����.
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