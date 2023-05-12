# libkchart
适用于Windows的可高度自定义K线控件

### 特点
* 内置股票K线相关图形以及指标计算.
* 流畅，内置图形绘制时以窗体右边为起点绘制, 这样在窗体缩放时不会出现抖动.
* 动态数据集, 使用二维表格存储图形数据, 其行列可随时添加.
* 抽象化图形类, 用户可以重写自己想要的图形(例如用于统计的图表).
* 软件基于 Gdiplus 作为绘图API, 其API已经过抽象, 您可以实现您自己的绘图引擎

### 用法
我们以内置K线图形为例, 当然也提供了完整的样例程序(目录:example/DayKLine.cpp)
1. 创建控件以及容纳图表的绘图区域.
```c++
    KChartWnd *wnd = new KChartWnd();
    wnd->CreateWin(NULL); // 创建Windows窗体.
    
    // 创建3个绘图区域, 这些区域会从上至下排列, 需要
    // 参数用于指定区域高度权重.
    GraphArea *areat = wnd->CreateArea(0.6f);
    GraphArea *aream = wnd->CreateArea(0.2f);
    GraphArea *areab = wnd->CreateArea(0.2f);
```
2. 创建数据表, 并将数据载入.
```c++
    DataSet& data = wnd->DataRef();
    // 添加数据列
    ColumnKey open  = data.AddCol("OPEN");
    ColumnKey high  = data.AddCol("HIGH");
    ColumnKey low   = data.AddCol("LOW");
    ColumnKey close = data.AddCol("CLOSE");
    
    ColumnKey close = data.AddCol("VOLUME");
    
    ColumnKey ma    = data.AddCol("MA5");
    ColumnKey macd  = data.AddCol("MACD");
    ColumnKey dif   = data.AddCol("DIF");
    ColumnKey dea   = data.AddCol("DEA");
    
    int row = 500; // 假设有 500 条数据
    // 添加 500 行
    data.AddRow(row);
    for (int i = 0; i < row; i++) {
        data.Set(open, i, /* val */);
        // ...
    }
    
    // ... 省略指标计算代码.
```
3. 添加图形.
```c++
    // 在主图区域添加K线图形, 需要用到数据集中的开高低收4列数据.
    // 注意这里传入 data 是用来查找需要的数据列索引, 并非直接
    // 将数据给到图形.
    areat->AddGraphics(new KLineGraph(data));
    areat->AddGraphics(new PolyLineGraph(data.FindCol("MA5")));
    areat->AddGraphics(new PolyLineGraph(data.FindCol("MA15")));

    aream->AddGraphics(new VolumeGraph(data));

    // MACD 需以 0 为中心轴.
    areab->SetCentralAxis(0);
    HistogramGraph *macdG = new HistogramGraph(data.FindCol("MACD"));
    macdG->FixWidth = 1; // MACD 是柱线图, 故需要将宽度设置为1.
    areab->AddGraphics(new PolyLineGraph(data.FindCol("DIF")));
    areab->AddGraphics(new PolyLineGraph(data.FindCol("DEA")));
    areab->AddGraphics(macdG);
    // ...
```
4. 最后一步.
```c++
    // 所有准备工作完成后, 只需要调用 Show 将窗体展示出来.
    wnd->Show(true);

    // 当然不要忘记 Windows 的消息循环哦!
    // ...
```
5. 运行结果.
![alt 运行结果](./img/1683901543715.png)
