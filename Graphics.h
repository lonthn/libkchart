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

    /// ��ͼ�α�����GraphArea��GraphArea����ColorList��
    /// ѡȡһ����ɫ��������, ��Ȼ����Ҳ�����Զ�����ɫ.
    virtual bool SetColor(Color color) {
        return false;
    }

    /// ��д Paint �Ի�����Ҫ��ͼ��
    virtual void Paint(
        GraphContext *gctx,
        const DrawData &data
    ) = 0;

    std::vector<ColumnKey> cids;
    DataType centralAxis;
};

/// K��ͼ����Ҫ�ṩ���ߵ���4������
/// �����ֱ���: OPEN, HIGH, LOW, CLOSE
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
        const DrawData &data
    ) override;
};


class PolyLineGraph : public Graphics {
public:
    Color LineColor;

    explicit PolyLineGraph(ColumnKey col)
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
        const DrawData &data
    ) override;
};

/// ��״ͼ
class HistogramGraph : public Graphics {
public:
    Color NormalColor;
    Color UpColor;
    Color DownColor;
    Scalar FixedWidth;

    explicit HistogramGraph(ColumnKey key)
    : Graphics(1)
    {
        UpColor     = 0xFFFF4A4A;
        DownColor   = 0xFF54FCFC;
        NormalColor = 0xFFFFFFFF;
        FixedWidth    = -1;

        cids[0] = key;
    }

    /// @brief ��ȡ��״ͼ��ɫ.
    virtual Color GetColor(const DrawData& data, int i)
    {
        return NormalColor;
    }

    /// @brief ��ȡ��״ͼ��ɫ, ��������������.
    virtual Color GetColorWithCA(const DrawData& data, int i)
    {
        DataType val = data.Get(cids[0], i);

        if (val > centralAxis)
            return UpColor;
        else if (val < centralAxis)
            return DownColor;

        return NormalColor;
    }

    void Paint(
        GraphContext *gctx,
        const DrawData& data
    ) override;
};

/// @brief ������״ͼ, ��Ҫ����2������(OPEN,CLOSE)������������
/// ͼ��ɫ, ����K��ͼ�еĳɽ���(��).
class VolumeGraph : public HistogramGraph {
public:
    explicit VolumeGraph(DataSet& data)
    : HistogramGraph(data.FindCol("VOLUME"))
    {
        cids.resize(3);
        cids[1] = data.FindCol("OPEN");
        cids[2] = data.FindCol("CLOSE");
    }

    Color GetColor(const DrawData& data, int i) override
    {
        DataType open = data.Get(cids[1], i);
        DataType close = data.Get(cids[2], i);

        if (open > close)
            return UpColor;
        else if (open < close)
            return DownColor;

        return NormalColor;
    }
};

}

#endif //LIBKCHART_GRAPHICS_H
