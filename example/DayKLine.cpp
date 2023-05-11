//
// Created by luo-zeqi on 2013/4/13.
//

#include "KChartWnd.h"

#include <vector>
#include <fstream>

using namespace kchart;


void LoadKLineData(const char *file, DataSet& data);

/// 计算指标数据并绑定图形. 注意：以下指标计算依赖 CLOSE 列数据.
void MA(DataSet& data, GraphArea *area, int num);
void MACD(DataSet& data, GraphArea *area);

void MessageLoop();

KChartWnd *wnd = NULL;

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
)
{
    wnd = new KChartWnd();
    wnd->CreateWin(NULL);


    /// 创建图形绘制区域
    GraphArea *mainArea = wnd->CreateArea(0.7f);
    GraphArea *volumeArea = wnd->CreateArea(0.2f);
    GraphArea *indiArea = wnd->CreateArea(0.2f);

    volumeArea->GetLeftAxis()->SetTransformFn(ToStringWithUnit);
    volumeArea->GetRightAxis()->SetTransformFn(ToStringWithUnit);


    /// 绑定数据
    DataSet& data = wnd->DataRef();
    LoadKLineData("SH000001.csv", data);


    /// 添加图形
    mainArea->AddGraphics(new KLineGraph(data));
    // 添加MA指标线
    MA(data, mainArea, 5);
    MA(data, mainArea, 10);
    MA(data, mainArea, 20);
    MA(data, mainArea, 30);
    MA(data, mainArea, 60);

    volumeArea->AddGraphics(new VolumeGraph(data));
    MACD(data, indiArea);


    wnd->Show(TRUE);
    MessageLoop();
    return 0;
}

void LoadKLineData(const char *file, DataSet& data)
{
//  symbol,date,open,high,low,close,volume,count
//  SH000001,202201040000,768.8720,803.7480,767.4470,803.7480,302111788,2928518397
//  SH000001,202201050000,768.8720,803.7480,767.4470,803.7480,302111788,2928518397
//  ...

    // 注意! 文件中的数据并不是真实市场行情数据, 不具备任何参考意义.
    std::ifstream ifs(file, std::ios::binary);
    assert(ifs.is_open());

    ColumnKey open  = data.AddCol("OPEN");
    ColumnKey high  = data.AddCol("HIGH");
    ColumnKey low   = data.AddCol("LOW");
    ColumnKey close = data.AddCol("CLOSE");
    ColumnKey vol   = data.AddCol("VOLUME");

    const int bufSize = 1024;
    char buf[bufSize];

    while (!ifs.eof())
    {
        ifs.getline(buf, bufSize - 1, '\n');

        Str line(buf);
        if (line.empty())
            continue;

        std::vector<Str> fields = line.split(',');
        if (fields[0] == "symbol")
            continue;

        int idx = data.AddRow();
        data[open] [idx] = fields[2].toFloat();
        data[high] [idx] = fields[3].toFloat();
        data[low]  [idx] = fields[4].toFloat();
        data[close][idx] = fields[5].toFloat();
        data[vol]  [idx] = fields[6].toFloat();
    }
}

void MA(DataSet& data, GraphArea *area, int num)
{
    ColumnKey close = data.FindCol("CLOSE");

    ColumnKey ma = data.AddCol("MA" + Str::ToString(num));
    int row = data.RowCount();

    DataType closeSum = 0;

    int i = 0;
    int off = num - 1;
    for (; i < off; i++)
    {
        closeSum += data[close][i];
        data[ma][i] = NAN;
    }

    for (; i < row; i++) {
        closeSum += data[close][i];
        data[ma][i] = closeSum / DataType(num);
        closeSum -= data[close][i-off];
    }

    area->AddGraphics(new PolyLineGraph(ma));
}

void MACD(DataSet& data, GraphArea *area)
{
    ColumnKey close = data.FindCol("CLOSE");

    ColumnKey dif   = data.AddCol("DIF");
    ColumnKey dea   = data.AddCol("DEA");
    ColumnKey macd  = data.AddCol("MACD");

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
        preEMA1 = preEMA1 * sc11 + data[close][i] * sc12;
        // EMA(n2) = 前一日EMA(n2)×(n2-1)/(n2+1) + 今日收盘价×2/(n2+1)
        preEMA2 = preEMA2 * sc21 + data[close][i] * sc22;
        // DIF = 今日EMA(n1) － 今日EMA(n2)
        DataType difv = preEMA1 - preEMA2;
        // 今日DEA = 前一日DEA×(n3-1)/(n3+1) + 今日DIF×2/(n3+1)
        preDEA = preDEA * sc31 + difv * sc32;

        data[dif] [i]  = difv;
        data[dea] [i]  = preDEA;
        data[macd][i] = (difv - preDEA) * 2;
    }

    area->SetCentralAxis(0);

    HistogramGraph *hg = new HistogramGraph(macd);
    hg->FixedWidth = 1;

    area->AddGraphics(hg);
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

        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_UP)
            {
                wnd->Zoom(1);
                wnd->Invalidate();
            }
            else if (msg.wParam == VK_DOWN)
            {
                wnd->Zoom(-1);
                wnd->Invalidate();
            }
        }
    }
}