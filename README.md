# libkchart
适用于Windows的可高度自定义K线控件

### 特点
* 超轻量级, 不含任何依赖库.
* 人性化API, 用法简单.
* 内置股票K线相关图形以及指标计算.
* 流畅，内置图形绘制时以窗体右边为起点绘制, 这样在窗体缩放时不会出现抖动.
* 动态数据集, 使用二维表格存储图形数据, 其行列可随时添加.
* 抽象化图形类, 用户可以重写自己想要的图形(例如用于统计的图表).
* 软件基于 Gdiplus 作为绘图API, 其API已经过抽象, 您可以实现您自己的绘图引擎.

### 构建
您可以通过 CMakeLists.txt 来构建 vs 的解决方案：
```shell
mkdir build
cd build
cmake -G <Visual Studio> -DCMAKE_BUILD_TYPE=Release ..
```

### 用法
我们以K线图中成交量柱图为例, 当然也提供了完整的样例程序 [example/DayKLine.cpp](./example/DayKLine.cpp)
1. 创建数据表, 并将数据载入, 下面代码中创建了一列股票成交量, 填充了500条数据.
```c++
    DataSet data;
    // 添加数据列
    ColumnKey vol = data.CreateCol("VOLUME");
    
    int row = 500; // 假设有 500 条数据
    // 添加 500 行
    data.AddRow(row);
    for (int i = 0; i < row; i++) {
        data.Set(vol, i, /* val */);
    }
```
2. 创建控件以及容纳图表的绘图区域, 绘图区域可同时添加多个(布局为上下结构), 添加多个区域是以权重参数来决定高度.
```c++
    KChartWnd *wnd = new KChartWnd(data);
    wnd->CreateWin(NULL); // 创建Windows窗体.
    
    // 参数用于指定区域高度权重.
    GraphArea *area = wnd->CreateArea(0.6f);
```
3. 添加图形, 用上一步添加的数据列创建一个柱状图形, 并添加至绘图区域中.   
tip: 单个绘图区域可同时添加多个不同图形进行叠加.
```c++
    // 也可以这样 new HistogramGraph(data.FindCol("VOLUME"));
    HistogramGraph *graph = new HistogramGraph(vol);
    area->AddGraphics(graph);
```
4. 最后一步, 所有准备工作完成后, 只需要调用 Show 将窗体展示出来.
```c++
    wnd->Show(true);

    // 当然不要忘记 Windows 的消息循环哦!
    // ...
```
