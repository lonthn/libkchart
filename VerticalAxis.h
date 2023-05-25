//
// Created by luo-zeqi on 2013/4/21.
//

#ifndef LIBKCHART_VERTICALAXIS_H
#define LIBKCHART_VERTICALAXIS_H

#include "graph/Scalar.h"
#include "graph/GraphContext.h"
#include "Def.h"
#include "Struct.h"
#include "StrUtils.h"

namespace kchart {

typedef CStringW (*TransformFn)(DataType);


static CStringW ToStringWithDigit(DataType val);
static CStringW ToStringWithUnit(DataType val);

class VerticalAxis
{
public:
    explicit VerticalAxis(Scalar width, bool aRight)
    : width_(width)
    , scaleColor_(0xFFFF4A4A)
    , alignToRight_(aRight)
    , transformFn_(ToStringWithDigit)
    { }
    virtual ~VerticalAxis() = default;

    void SetScaleFormatter(TransformFn fn) {
        transformFn_ = fn;
    }

    Scalar GetWidth() const {
        return width_;
    }

    virtual void OnSetScales(const DataRows &scales);
    virtual void OnPaint(
        GraphContext *ctx,
        DrawData& data,
        Scalar offY
    );

private:
    bool     alignToRight_;
    Scalar   width_;
    Color    scaleColor_;

    TransformFn transformFn_;

    DataRows scales_;
    std::vector<CStringW> strScales_;
};

static CStringW ToStringWithDigit(DataType val)
{
    return DoubleToStr(val, 2);
}

static CStringW ToStringWithUnit(DataType val)
{
    if (val < 10000)
        return DoubleToStr(val, 2);
    else if (val < 100000000.0)
        return DoubleToStr(val / 10000.0f, 2) + L"Эђ";
    else
        return DoubleToStr(val / 100000000.0f, 2) + L"вк";
}

}

#endif //LIBKCHART_VERTICALAXIS_H
