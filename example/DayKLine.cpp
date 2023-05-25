//
// Created by luo-zeqi on 2013/4/13.
//

#include "KChartWnd.h"

#include <vector>
#include <fstream>

using namespace kchart;


void LoadKLineData(const char *file, DataSet& data);

/// ����ָ�����ݲ���ͼ��. ע�⣺����ָ��������� CLOSE ������.
std::vector<Graphics *> MA(DataSet& data, const std::vector<int>& nums);
std::vector<Graphics *> MACD(DataSet& data);

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

    GraphArea *mainArea = wnd->CreateArea(0.6f);
    GraphArea *ind1Area = wnd->CreateArea(0.2f);
    GraphArea *ind2Area = wnd->CreateArea(0.2f);

    // ���ڳɽ������ֽϴ�, ��չʾ�̶�ʱ�����ô���λ����ʽ.
    ind1Area->GetLeftAxis()->SetScaleFormatter(ToStringWithUnit);
    ind1Area->GetRightAxis()->SetScaleFormatter(ToStringWithUnit);

    // ���ļ��е���ʷ��K���ݼ��ص� DataSet ��,
    DataSet& data = wnd->DataRef();
    LoadKLineData("SZ000001.csv", data);

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

void LoadKLineData(const char *file, DataSet& data)
{
//  symbol,date,open,high,low,close,volume
//  SH000001,20220104,768.8720,803.7480,767.4470,803.7480,302111788,2928518397
//  SH000001,20220105,768.8720,803.7480,767.4470,803.7480,302111788,2928518397
//  ...

    std::ifstream ifs(file, std::ios::binary);
    assert(ifs.is_open());

    ColumnKey open  = data.AddCol("OPEN");
    ColumnKey high  = data.AddCol("HIGH");
    ColumnKey low   = data.AddCol("LOW");
    ColumnKey close = data.AddCol("CLOSE");
    ColumnKey vol   = data.AddCol("VOLUME");

    const int bufSize = 1024;
    char buf[bufSize];

    // Skip the header.
    ifs.getline(buf, bufSize - 1, '\n');

    while (!ifs.eof())
    {
        ifs.getline(buf, bufSize - 1, '\n');

        CStringA line(buf);
        if (line.IsEmpty())
            continue;

        std::vector<CStringA> fields;
        StrSplit(line.GetString(), ",", true, fields);

        char* endptr = nullptr;

        int idx = data.AddRow();
        data[open ][idx] = std::strtof(fields[2], &endptr);
        data[high ][idx] = std::strtof(fields[3], &endptr);
        data[low  ][idx] = std::strtof(fields[4], &endptr);
        data[close][idx] = std::strtof(fields[5], &endptr);
        data[vol  ][idx] = std::strtof(fields[6], &endptr);
    }
}

std::vector<Graphics *> MA(DataSet& data, const std::vector<int>& nums)
{
    int row = data.RowCount();
    ColumnKey close = data.FindCol("CLOSE");

    auto ma = [&](int n) {
        ColumnKey ma = data.AddCol("MA" + std::to_string(n));

        DataType closeSum = 0;
        int i = 0;
        int off = n - 1;
        for (; i < off; i++)
        {
            closeSum += data[close][i];
            data[ma][i] = NAN; // NAN ��ʾ������, ���û���.
        }

        for (; i < row; i++) {
            closeSum += data[close][i];
            // ����MA(n) = ǰn-1���Լ��������̼�֮�� / n
            data[ma][i] = closeSum / DataType(n);
            closeSum -= data[close][i-off];
        }

        return new PolyLineGraph(ma);
    };

    std::vector<Graphics *> graph;
    for (int n : nums)
        graph.push_back(ma(n));

    return graph;
}

std::vector<Graphics *> MACD(DataSet& data)
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
        // EMA(n1) = ǰһ��EMA(n1)��(n1-1)/(n1+1) + �������̼ۡ�2/(n1+1)
        preEMA1 = preEMA1 * sc11 + data[close][i] * sc12;
        // EMA(n2) = ǰһ��EMA(n2)��(n2-1)/(n2+1) + �������̼ۡ�2/(n2+1)
        preEMA2 = preEMA2 * sc21 + data[close][i] * sc22;
        // DIF = ����EMA(n1) �� ����EMA(n2)
        DataType difv = preEMA1 - preEMA2;
        // ����DEA = ǰһ��DEA��(n3-1)/(n3+1) + ����DIF��2/(n3+1)
        preDEA = preDEA * sc31 + difv * sc32;

        data[dif] [i]  = difv;
        data[dea] [i]  = preDEA;
        data[macd][i] = (difv - preDEA) * 2;
    }

    std::vector<Graphics *> graphics;

    HistogramGraph *hg = new HistogramGraph(macd);
    hg->FixedWidth = 1; // macd ʹ�õ�������ͼ

    graphics.push_back(hg);
    graphics.push_back(new PolyLineGraph(dif));
    graphics.push_back(new PolyLineGraph(dea));

    return graphics;
}

void MessageLoop()
{
    MSG msg;
    while (wnd->Handle() && ::GetMessageA(&msg, NULL, 0, 0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);

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