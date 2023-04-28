//
// Created by luo-zeqi on 2013/4/17.
//

#ifndef LIBKCHART_GRAPHICS_H
#define LIBKCHART_GRAPHICS_H

#include "Def.h"
#include "Struct.h"
#include "graph/GraphContext.h"

namespace kchart {

class Graphics
{
public:
    explicit Graphics(int colNum)
    : centralAxis(NAN) {
        cids.resize(colNum, 0);
    }

    /// 在图形被加入GraphArea后，GraphArea会在ColorList中
    /// 选取一个颜色进行设置, 当然我们也可以自定义颜色.
    virtual bool SetColor(Color color) {
        return false;
    }

    virtual void Paint(
        GraphContext *gctx,
        DrawData& data
    ) = 0;

    std::vector<ColumnKey*> cids;
    DataType centralAxis;
};

class KLineGraph : public Graphics
{
    enum Cid
    {
        Open = 0,
        High,
        Low,
        Close,
    };

public:
    Color UpColor;
    Color NormalColor;
    Color DownColor;
    Color TextColor;

    int Digit;

    explicit KLineGraph(DataSet& data)
      : Graphics(4)
      , Digit(2)
    {
        UpColor     = 0xFFFF4A4A;
        DownColor   = 0xFF54FCFC;
        NormalColor = 0xFFFFFFFF;
        TextColor   = 0xFFFFFFFF;

        cids[Open]  = data.FindCol("OPEN");
        cids[High]  = data.FindCol("HIGH");
        cids[Low]   = data.FindCol("LOW");
        cids[Close] = data.FindCol("CLOSE");
    }

    void Paint(
        GraphContext *gctx,
        DrawData& data
    ) override;
};

class VolumeGraph : public Graphics {
public:
    Color UpColor;
    Color NormalColor;
    Color DownColor;

    explicit VolumeGraph(DataSet& data)
    : Graphics(3)
    {
        UpColor     = 0xFFFF4A4A;
        DownColor   = 0xFF54FCFC;
        NormalColor = 0xFFFFFFFF;

        cids[0] = data.FindCol("OPEN");
        cids[1] = data.FindCol("CLOSE");
        cids[2] = data.FindCol("VOLUME");
    }

    void Paint(
        GraphContext *gctx,
        DrawData& data
    ) override;
};

class PolyLineGraph : public Graphics {
public:
    Color LineColor;

    explicit PolyLineGraph(ColumnKey* col)
    : Graphics(1)
    , LineColor(0xFFFFFFFF)
    {
        cids[0] = col;
    }

    bool SetColor(Color color) override
    {
        LineColor = color;
        return true;
    }

    void Paint(
        GraphContext *gctx,
        DrawData& data
    ) override;
};

/// 柱状图
///
class HistogramGraph : public Graphics {
public:
    Color NormalColor;
    Color UpColor;
    Color DownColor;

    explicit HistogramGraph(ColumnKey *key)
    : Graphics(1)
    {
        UpColor     = 0xFFFF4A4A;
        DownColor   = 0xFF54FCFC;
        NormalColor = 0xFFFFFFFF;

        cids[0] = key;
    }

    void Paint(
        GraphContext *gctx,
        DrawData& data
    ) override;
};

}

#endif //LIBKCHART_GRAPHICS_H
