//
// Created by luo-zeqi on 2013/4/17.
//

#ifndef LIBKCHART_GRAPHICS_H
#define LIBKCHART_GRAPHICS_H

#include "Def.h"
#include "Struct.h"
#include "graph/GraphContext.h"

namespace kchart {

class Graphics {
public:
  explicit Graphics(int colNum)
      : centralAxis(KC_INVALID_DATA)
      , Digit(2)
      , NormalColor(0xFFF0F0F0)
      , ZeroOrigin(false) {
    cids.resize(colNum, 0);
  }

  virtual ~Graphics() = default;

  virtual void ChangeTheme(bool white) {
    NormalColor = white ? 0xFF101010 : 0xFFF0F0F0;
  }

  /// ��ͼ�α�����GraphArea��GraphArea����ColorList��
  /// ѡȡһ����ɫ��������, ��Ȼ����Ҳ�����Զ�����ɫ.
  virtual bool SetColor(Color color) {
    return false;
  }

  /// @brief ��ȡͼ����ɫ.
  virtual Color GetColor(const DrawData &data, int i) {
    return NormalColor;
  }

  /// @brief ��ȡͼ����ɫ, ��������������.
  virtual Color GetColorWithCA(const DrawData &data, int i) {
    return NormalColor;
  }

  /// ��д Paint �Ի�����Ҫ��ͼ��
  virtual void Paint(
      GraphContext *gctx,
      const DrawData &data
  ) = 0;

  std::vector<ColumnKey> cids;
  DataType centralAxis;

  bool  ZeroOrigin;
  int   Digit;
  Color NormalColor;
};

/// K��ͼ����Ҫ�ṩ���ߵ���4������
/// �����ֱ���: OPEN, HIGH, LOW, CLOSE
class KLineGraph : public Graphics {

  enum Cid { Open = 0, High, Low, Close, };

public:
  Color UpColor;
  Color DownColor;
  Color TextColor;

  explicit KLineGraph(DataSet &data)
      : Graphics(4) {
    UpColor   = 0xFFFF4A4A;
    DownColor = 0xFF54FCFC;
    TextColor = NormalColor;

    cids[Open]  = data.FindCol("OPEN");
    cids[High]  = data.FindCol("HIGH");
    cids[Low]   = data.FindCol("LOW");
    cids[Close] = data.FindCol("CLOSE");
  }

  void ChangeTheme(bool white) override {
    Graphics::ChangeTheme(white);
    TextColor = NormalColor;
    DownColor = white ? 0xFF10AB62 : 0xFF54FCFC;
  }

  void Paint(
      GraphContext *gctx,
      const DrawData &data
  ) override;
};


class PolyLineGraph : public Graphics {
public:
  int LineWidth;

  explicit PolyLineGraph(ColumnKey col, int lw = 1)
      : Graphics(1)
      , LineWidth(lw) {
    cids[0] = col;
  }

  // ����Ҫ�л�
  void ChangeTheme(bool) override {}

  bool SetColor(Color color) override {
    NormalColor = color;
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
  Color  UpColor;
  Color  DownColor;
  Scalar FixedWidth;

  explicit HistogramGraph(ColumnKey key, Scalar fw = -1)
      : Graphics(1)
      , FixedWidth(fw) {
    ZeroOrigin = true;
    UpColor = 0xFFFF4A4A;
    DownColor = 0xFF54FCFC;

    cids[0] = key;
  }

  void ChangeTheme(bool white) override {
    Graphics::ChangeTheme(white);
    DownColor = white ? 0xFF10AB62 : 0xFF54FCFC;
  }

  Color GetColorWithCA(const DrawData &data, int i) override {
    DataType val = data.Get(cids[0], i);

    if (val > centralAxis)
      return UpColor;
    else if (val < centralAxis)
      return DownColor;

    return NormalColor;
  }

  void Paint(
      GraphContext *gctx,
      const DrawData &data
  ) override;
};

/// @brief ������״ͼ, ��Ҫ����2������(OPEN,CLOSE)������������
/// ͼ��ɫ, ����K��ͼ�еĳɽ���(��).
class VolumeGraph : public HistogramGraph {
public:
  explicit VolumeGraph(DataSet &data)
      : HistogramGraph(data.FindCol("VOLUME")) {
    openKey_ = data.FindCol("OPEN");
    closeKey_ = data.FindCol("CLOSE");
  }

  Color GetColor(const DrawData &data, int i) override {
    DataType open = data.Get(openKey_, i);
    DataType close = data.Get(closeKey_, i);

    if (open < close)
      return UpColor;
    else if (open > close)
      return DownColor;

    return NormalColor;
  }

private:
  ColumnKey openKey_;
  ColumnKey closeKey_;
};

}

#endif //LIBKCHART_GRAPHICS_H
