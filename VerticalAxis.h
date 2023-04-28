//
// Created by luo-zeqi on 2013/4/21.
//

#ifndef LIBKCHART_VERTICALAXIS_H
#define LIBKCHART_VERTICALAXIS_H

#include "graph/Scalar.h"
#include "graph/GraphContext.h"
#include "Def.h"
#include "Struct.h"

namespace kchart {

typedef WStr (*TransformFn)(DataType);


static WStr ToStringWithDigit(DataType val);
static WStr ToStringWithUnit(DataType val);

class VerticalAxis
{
public:
    explicit VerticalAxis(Scalar width, bool aRight)
    : width_(width)
    , scaleColor_(0xFFFF4A4A)
    , alignToRight_(aRight)
    , transformFn_(ToStringWithDigit)
    { }

    void SetTransformFn(TransformFn fn) {
        transformFn_ = fn;
    }

    Scalar GetWidth() const {
        return width_;
    }

    virtual void OnSetScales(const DataRows &scales);

    virtual void OnPaint(GraphContext *ctx, DrawData& data,
                         Scalar height);

private:
    bool     alignToRight_;
    Scalar   width_;
    Color    scaleColor_;

    TransformFn transformFn_;

    DataRows scales_;
    std::vector<WStr> strScales_;
};

static WStr ToStringWithDigit(DataType val)
{
    return WStr::ToString(val, 2);
}

static WStr ToStringWithUnit(DataType val)
{
    if (val < 10000)
        return WStr::ToString(val, 2);
    else if (val < 100000000.0)
        return WStr::ToString(val/10000.0f, 2) + L"Эђ";
    else
        return WStr::ToString(val/100000000.0f, 2) + L"вк";
}

}

#endif //LIBKCHART_VERTICALAXIS_H
