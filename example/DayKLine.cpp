//
// Created by luo-zeqi on 2013/4/13.
//

#include "KChartWnd.h"

#include <vector>
#include <fstream>

using namespace kchart;

void LoadKLineData(const char *file, DataSet& data);

/// 计算指标数据并绑定图形. 注意：数据集中需包含 CLOSE 数据
void MA(DataSet& data, GraphArea *area, int num);
void MACD(DataSet& data, GraphArea *area);

void MessageLoop();

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
)
{
    KChartWnd *wnd = new KChartWnd();
    wnd->CreateWin(NULL);

    // 创建图形绘制区域
    GraphArea *mainArea = wnd->CreateArea(0.7f);
    GraphArea *volumeArea = wnd->CreateArea(0.2f);
    GraphArea *indiArea = wnd->CreateArea(0.2f);

    volumeArea->GetLeftAxis()->SetTransformFn(ToStringWithUnit);
    volumeArea->GetRightAxis()->SetTransformFn(ToStringWithUnit);

    // 添加数据
    DataSet& data = wnd->DataRef();
    LoadKLineData("SH000001.csv", data);

    // 添加图形
    mainArea->AddGraphics(new KLineGraph(data));
    MA(data, mainArea, 5);
    MA(data, mainArea, 10);
    MA(data, mainArea, 20);
    MA(data, mainArea, 30);
    MA(data, mainArea, 60);

    volumeArea->AddGraphics(new VolumeGraph(data));

    MACD(data, indiArea);

    wnd->Show(TRUE);
    UpdateWindow(wnd->Handle());

    MessageLoop();
    return 0;
}

void LoadKLineData(const char *file, DataSet& data)
{
//  symbol,date,open,high,low,close,volume,count
//  SH000001,202201040000,768.8720,803.7480,767.4470,803.7480,302111788,2928518397
    std::ifstream ifs(file, std::ios::binary);
    assert(ifs.is_open());

    ColumnKey *open  = data.AddCol("OPEN");
    ColumnKey *high  = data.AddCol("HIGH");
    ColumnKey *low   = data.AddCol("LOW");
    ColumnKey *close = data.AddCol("CLOSE");
    ColumnKey *vol   = data.AddCol("VOLUME");

    const int bufSize = 1024;
    char buf[bufSize];

    int row = 0;
    while (!ifs.eof())
    {
        ifs.getline(buf, bufSize - 1, '\n');

        Str line(buf);
        if (line.empty())
            continue;

        std::vector<Str> fields = line.split(',');
        if (fields[0] == "symbol")
            continue;

        data.AddRow(1);

        data[open->index] [row] = fields[2].toFloat();
        data[high->index] [row] = fields[3].toFloat();
        data[low->index]  [row] = fields[4].toFloat();
        data[close->index][row] = fields[5].toFloat();
        data[vol->index]  [row] = fields[6].toFloat();

        row++;
    }
}

void MA(DataSet& data, GraphArea *area, int num)
{
    ColumnKey* close = data.FindCol("CLOSE");

    ColumnKey* ma = data.AddCol("MA" + Str::ToString(num));
    int row = data.RowCount();

    DataType closeSum = 0;
    int i = 0;
    for (; i < num; i++)
    {
        closeSum += data[close->index][i];
        data[ma->index][i] = NAN;
    }

    i--;

    data[ma->index][i] = closeSum / DataType(num);

    for (i = i+1; i < row; i++) {
        closeSum += data[close->index][i];
        closeSum -= data[close->index][i-num];
        data[ma->index][i] = closeSum / DataType(num);
    }

    area->AddGraphics(new PolyLineGraph(ma));
}

void MACD(DataSet& data, GraphArea *area)
{
    ColumnKey *close = data.FindCol("CLOSE");

    ColumnKey *dif   = data.AddCol("DIF");
    ColumnKey *dea   = data.AddCol("DEA");
    ColumnKey *macd  = data.AddCol("MACD");

    DataType n1 = 12;
    DataType sc11 = (n1 - 1) / (n1 + 1);
    DataType sc12 = 2/(n1 + 1);
    DataType n2 = 26;
    DataType sc21 = (n2 - 1) / (n2 + 1);
    DataType sc22 = (2)/(n2 + 1);
    DataType n3  = 9;
    DataType sc31 = (n3 - 1) / (n3 + 1);
    DataType sc32 = (2)/(n3 + 1);

    DataType preEMA1 = 0, preEMA2 = 0, preDEA = 0;
    for (int i = 0; i < data.RowCount(); i++)
    {
        // EMA(n1) = 前一日EMA(n1)×(n1-1)/(n1+1) + 今日收盘价×2/(n1+1)
        preEMA1 = preEMA1 * sc11 + data[close->index][i] * sc12;
        // EMA(e2) = 前一日EMA(e2)×(n2-1)/(n2+1) + 今日收盘价×2/(n2+1)
        preEMA2 = preEMA2 * sc21 + data[close->index][i] * sc22;
        // DIF = 今日EMA(n1) － 今日EMA(n2)
        DataType difv = preEMA1 - preEMA2;
        // 今日DEA = 前一日DEA×(n3-1)/(n3+1) + 今日DIF×2/(n3+1)
        preDEA = preDEA * sc31 + difv * sc32;

        data[dif->index][i]  = difv;
        data[dea->index][i]  = preDEA;
        data[macd->index][i] = (difv - preDEA) * 2;
    }

    area->SetCentralAxis(0);
    area->AddGraphics(new HistogramGraph(macd));
    area->AddGraphics(new PolyLineGraph(dif));
    area->AddGraphics(new PolyLineGraph(dea));
}

void MessageLoop()
{
    MSG msg;
    while (::GetMessageA(&msg, NULL, 0, 0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);

        if (msg.message == WM_QUIT
         || msg.message == WM_CLOSE)
            break;
    }
}