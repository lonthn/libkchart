//
// Created by luo-zeqi on 2013/4/17.
//

#ifndef WINCTRL_SHAPE_H
#define WINCTRL_SHAPE_H

#include "Def.h"
#include "Struct.h"
#include "graph/GraphContext.h"
#include "Graphics.h"

namespace kchart {

class KChartWnd;
class VerticalAxis;

class GraphArea
{
public:
    virtual ~GraphArea();

    virtual DataType GetCentralAxis();

    virtual void SetCentralAxis(DataType n);

    virtual bool AddGraphics(Graphics *graph);

    virtual VerticalAxis *GetLeftAxis() {
        return lAxis_;
    }
    virtual VerticalAxis *GetRightAxis() {
        return rAxis_;
    }

protected:
    explicit GraphArea(KChartWnd *panel,
                       VerticalAxis *la,
                       VerticalAxis *ra);

    void SetWeight(float weight) {
        weight_ = weight;
    }

    float GetWeight() const {
        return weight_;
    }

    Scalar GetContentHeight() const {
        return bounds_.bottom - bounds_.top;
    }

    void SetBounds(const Rect& bounds) {
        bounds_ = bounds;
    }

    Rect GetBounds() const {
        return bounds_;
    }

    const DataRows& GetScales() {
        return scales_;
    }

    virtual void MinMaxData();
    virtual void UpdateScales();

    virtual void OnFitIdx(int begin, int end);

    virtual void OnPaint(GraphContext *ctx);
    virtual void OnPaintGraph(GraphContext *ctx, DrawData& data);

protected:
    KChartWnd *panel_;
    float     weight_;

    Rect bounds_;
    Rect graphArea_;

    int begin_;
    int end_;
    DataType cacheMin_;
    DataType cacheMax_;
    bool validCache_;

    Color scaleLineColor_;
    int colorIdx_;
    std::vector<Color> colorList_;
    DataRows scales_;
    VerticalAxis *lAxis_;
    VerticalAxis *rAxis_;
    DataType centralAxis_;

    std::vector<Graphics*> graphics_;

    friend KChartWnd;
    friend VerticalAxis;
};

}

#endif //WINCTRL_SHAPE_H
