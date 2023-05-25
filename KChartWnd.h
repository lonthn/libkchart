//
// Created by luo.zeqi on 2013/4/13.
//

#ifndef WINCTRL_KLINEWND_H
#define WINCTRL_KLINEWND_H

#include "GraphArea.h"
#include "Graphics.h"
#include "Struct.h"
#include "graph/GdiPlusGC.h"
#include "VerticalAxis.h"

#include <Windows.h>

class WndProcThunk;

namespace kchart {

/// @brief ����ͼ��Ĵ�����, ����Ϊ������������, Ҳ������
/// �����е�һ���ؼ�.
class KChartWnd
{
public:
    KChartWnd();
    virtual ~KChartWnd();

    HWND Handle() {
        return handle_;
    }

    DataSet& DataRef() {
        return data_;
    }

    void SetCrosshairColor(Color color) {
        crosshairColor_ = color;
    }
    Color GetCrosshairColor() const {
        return crosshairColor_;
    }

    /// @brief ������õĺ���, ʹ��WindowsAPI��������
    /// hParent ��������ľ��, ��Ϊ��.
    virtual bool CreateWin(HWND hParent = nullptr);

    virtual Scalar   Width() const;
    virtual Scalar   Height() const;
    virtual Size     GetSize() const;
    virtual Point    GetLocation() const;
    virtual Rect     GetBounds() const;
    virtual std::string GetTitle() const;

    virtual void SetSize(const Size& size);
    virtual void SetLocation(const Point& point);
    virtual void SetBounds(const Rect& rect);
    virtual void SetTitle(const std::string& str);

    virtual void Show(bool show);
    /// @brief ʹ��������ʧЧ, �Ӷ������ػ��¼�
    /// ��ʹ���淢���˱仯, ����Ҫ���ô˺��������ػ�.
    virtual void Invalidate();

    virtual Rect GetAreaBounds();

    /// @brief ����ͼ������, ���Դ������(���½ṹ),
    /// ÿ�������Ƕ���������ϵ, percent ����ָ����
    /// ��Ȩ��(�ڲ�����Ȩ�ط���߶�).
    virtual GraphArea *CreateArea(float percent);

    /// @brief ��ͼ�ν�������.
    /// factor �����Ŵ�, ������С.
    virtual void Zoom(int factor);

public:
    static std::string ClassName() {
        return "WINEX_CLS_KChart";
    }

protected:
    virtual void OnSetCrosshairPoint(Point point);

    LRESULT CALLBACK OnMessage(
        UINT msg, WPARAM wParam, LPARAM lParam
    );

    virtual LRESULT OnCreate (WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnSize   (WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnPaint  (WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnLBtnDown(WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);

private:
    void FillDrawData(GraphArea * area, DrawData& data);
    void Layout();
    void FitNewWidth();

private:
    HWND handle_;

    Color borderColor_;
    Size size_;

    // �Ƿ���ʾ��������(������������).
    bool lvVisible_;
    bool rvVisible_;
    Scalar vAxisWidth_;
    std::vector<VerticalAxis*> lvAxis_;
    std::vector<VerticalAxis*> rvAxis_;
    std::vector<GraphArea*> areas_;

    bool  crosshairEnable_;
    bool  crosshairVisible_;
    Color crosshairColor_;
    Point crosshairPoint_;

    // �����ܹ���ʾ�����ݷ�Χ, �����Χ�ǿ��Գ��� DataSet
    // ���ִ������.
    int beginIdx_;
    int endIdx_;
    float sWidth_;
    DataSet data_;

    WndProcThunk *procThunk_;
    GdiPlusGC *gcContext_;
};

}

#endif // WINCTRL_KLINEWND_H
