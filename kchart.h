// MIT License
//
// Copyright (c) 2023 luo-zeqi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef LIBKCHART_H
#define LIBKCHART_H

#include <cstdint>
#include <Windows.h>
#include <gdiplus.h>
#include <atlstr.h>

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

// ͨ������������չʾ�����ݲ����ܴﵽ int64 �����ֵ
// ����������������ʾ�����������.
#define KC_INVALID_DATA INT64_MAX //0x7FFFFFFFFFFFFFFF

typedef uint32_t Color;

#define ColorGetA(color)    (((color) >> 24u) & 0xffu)
#define ColorGetR(color)    (((color) >> 16u) & 0xffu)
#define ColorGetG(color)    (((color) >>  8u) & 0xffu)
#define ColorGetB(color)    ((color) & 0xffu)

inline Color MakeColor(uint8_t r,
  uint8_t g,
  uint8_t b,
  uint8_t a = 255) {
  return ((Color)(
      (((uint32_t)a) << 24u) |
      (((uint32_t)r) << 16u) |
      (((uint32_t)g) << 8u) |
      b)
      );
}


namespace kchart {

typedef int  GraphicsId;

typedef int64_t  DataType;
typedef uint32_t Color;

typedef std::vector<DataType>  DataRows;
typedef std::vector<DataRows>  DataCols;

typedef CStringW(*TransformFn)(DataType, int, int);


// �������������չʾ�����ǿ���û���㹻�Ŀռ䣬���Կ���
// ���Դ��ϵ�λ.
extern CStringW ToStrWithUnit(
  DataType val,
  int precision,
  int decimals
);

extern CStringW DataToStr(
  DataType val,
  int precision,
  int decimals
);

extern void StrSplit(
  const char* str,
  const char* delis,
  bool ignore_space,
  std::vector<std::string>& out
);

/* Scalar ----------------------------------------------------------------------*/

// ���ں���ͼ�θ�ά�Ĳ���
typedef int Scalar;

struct Size {
  Scalar width;
  Scalar height;
};

struct Point {
  Scalar x;
  Scalar y;
};

struct Rect {
  Scalar left;
  Scalar top;
  Scalar right;
  Scalar bottom;

  inline Scalar Width() const {
      return right - left;
  }

  inline Scalar Height() const {
      return bottom - top;
  }

  inline Point Point() const {
      return { left, top };
  }

  inline Size Size() const {
      return { Width(), Height() };
  }
};


/* GraphContext ----------------------------------------------------------------------*/
typedef int FontId;
static FontId FontId_Arial = 0;
static FontId FontId_WRYH = 1;

class GraphContext {
public:
  std::vector<CStringW> fontFamilies_;

  GraphContext() {
    fontFamilies_.resize(2);
    fontFamilies_[FontId_Arial] = L"Arial";
    fontFamilies_[FontId_WRYH] = L"Microsoft YaHei UI";
  }

  virtual void AllocBuffer(const Size& size) { }
  virtual void SwapBuffer(void* native) { }

public:
  // ����ԭ��λ��.
  virtual Point Translate(const Point& point) { return { 0, 0 }; }
  virtual void SetTranslate(const Point& point) {}

  virtual void SetFont(FontId fontId, int size) {}
  virtual void SetColor(Color color) {}
  virtual void SetLineWidth(int width) {}

  /**
   * ��Ⱦ����, DrawRect ������ĵ�, FillRect ����ʵ��.
   * @param r
   */
  virtual void DrawRect(const Rect& r) {}
  virtual void DrawRect(
    Scalar l, Scalar t, Scalar r, Scalar b
  ) {}
  virtual void FillRect(const Rect& r) {}
  virtual void FillRect(
    Scalar l, Scalar t, Scalar r, Scalar b
  ) {}

  /**
   * �ṩ������, ��һ���߶�.
   * @param a
   * @param b
   */
  virtual void DrawLine(
    const Point& a, const Point& b
  ) {}
  virtual void DrawLine(
    Scalar ax, Scalar ay, Scalar bx, Scalar by
  ) {}

  /**
   * ��һ���Զ�������,
   * �ṩ����2�������ϵĵ㣬����Щ�������������ͼ
   * @param pointNum ָ���������.
   * @example
   * // ʹ��ʱ��Ҫ�õ�3���������ڵ���EndPolyLineʱ
   * �����е�������Ⱦ�ڻ�����:
   *  ctx.BeginPolyLine(5);
   *  for (int i = 0; i < 5; i++) {
   *    ctx.AddPolyLine({x, y});
   *  }
   *  ctx.EndPolyLine();
   */
  virtual void BeginPolyLine(int pointNum) {}
  virtual void AddPolyLine(const Point& p) {}
  virtual void EndPolyLine() {}

  /**
   * ���ַ��� str ��Ⱦ�ڻ�����
   * @param str
   * @param p �ı����Ͻǵ�λ��.
   */
  virtual void DrawStr(const CStringW& str, const Point& p) {}
  // Ҫ�����ı�, ͨ��������Ҫ �����ı��Ŀ�ߣ�
  // ���ǻῼ�� ���� �� size ����.
  virtual Size MeasureStr(const CStringW& str) { return { 0, 0 }; }
};

/* GdiPlusGC ----------------------------------------------------------------------*/
class GdiPlusGC : public GraphContext {
public:
  GdiPlusGC();

  virtual ~GdiPlusGC();

  void AllocBuffer(const Size& size) override;
  void SwapBuffer(void* native) override;

public:
  Point Translate(const Point& point) override;
  void SetTranslate(const Point& point) override;
  void SetFont(FontId fontId, int size) override;
  void SetColor(Color color) override;
  void SetLineWidth(int width) override;

  void DrawRect(const Rect& r) override;
  void DrawRect(
    Scalar l, Scalar t, Scalar r, Scalar b
  ) override;

  void FillRect(const Rect& r) override;
  void FillRect(
    Scalar l, Scalar t, Scalar r, Scalar b
  ) override;

  void DrawLine(
    const Point& a, const Point& b
  ) override;
  void DrawLine(
    Scalar ax, Scalar ay, Scalar bx, Scalar by
  ) override;

  void BeginPolyLine(int pointNum) override;
  void AddPolyLine(const Point& p) override;
  void EndPolyLine() override;

  void DrawStr(const CStringW& str, const Point& p) override;
  Size MeasureStr(const CStringW& str) override;

private:
  HDC memDC_;
  HBITMAP memBitmap_;

  Color color_;

  Gdiplus::Graphics* gdiGraph_;
  Gdiplus::Pen* gdiPen_;
  Gdiplus::SolidBrush* gdiBrush_;
  Gdiplus::Font* gdiFont_;

  int pointNum_;
  std::vector<Gdiplus::Point> pointCache_;

  Size cacheSize_;

  Point offset_;

  std::map<uint64_t, Gdiplus::Font*> fontCache_;
};

/* DataSet ------------------------------------------------------------*/
struct Setter {
  int fieldOffset;
  typedef void (*Function)(kchart::DataType*, void*);
  Function fn;

  Setter(int off, Function f)
    : fieldOffset(off)
    , fn(f) {
  }

  void operator () (kchart::DataType* d, void* s) const {
    fn(d, ((char*)s) + fieldOffset);
  }
};

#define MAKE_SETTER(s, m, f) \
  new Setter(offsetof(s, m), f)

// �����������Ϣ
typedef struct ColumnInfo {
  CStringW name;
  // ����ʹ�����ʹ洢������ͨ����������
  // �洢������.
  int precision;
  int index;

  Setter* setter;
} *ColumnKey;

struct Index2 {
  int col;
  int row;
};

// @brief ԭʼ���ݼ�, ��2ά������ʽ�洢ͼ�ζ�Ӧ��ԭʼ
// �����ҿ���ʱ����.
// @details ͨ�����ǻᴴ��һ��ͼ�λ����Զ���ͼ��, ����
// ������ͼ����������ݶ�Ӧ�� ColumnKey(ͨ��AddCol/FindCol���).
struct DataSet {

  typedef std::function<void(DataSet& data, int)> ObserverFn;

  DataSet();

  // @brief �����, ���ṩ����.
  ColumnKey CreateCol(const std::string& name);
  ColumnKey CreateCol(const std::string& name, int precision);
  ColumnKey CreateCol(const std::string& name, Setter* setter);
  ColumnKey CreateCol(const std::string& name, int precision, Setter* setter);
  // @brief ���һ��.
  int AddRow();
  // @brief �����, ��ָ������.
  void AddRow(int n);

  // @brief ������������ ColumnKey(������, ���ڻ�ȡ
  // ��Ӧ�е�����).
  ColumnKey FindCol(const std::string& name);

  inline int RowCount() const { return rowCount; }
  inline int ColCount() const { return (int)raw.size(); }
  inline int LastRow() const { return rowCount - 1; }
  inline int Empty() const { return rowCount == 0; }

  DataType Get(int col, int row);
  DataType Get(const Index2& idx);
  DataType Get(ColumnKey col, int row);
  void Set(int col, int row, DataType val);
  void Set(const Index2& idx, DataType val);
  void Set(ColumnKey col, int row, DataType val);
  DataRows& Get(ColumnKey col);
  // @note ͨ���ú�����õ�����������������д�벢
  // ���ᴥ���۲���.
  DataRows& operator[](int col);

  void FillRow(void* src, int idx) {
    for (auto& item : columns) {
      ColumnKey col = &item.second;
      if (col->setter)
        (*col->setter)(&(raw[col->index][idx]), src);
    }
  }

  // @brief ������ݼ���, ��Notify�����ú��ͨ��ObserverFn
  // ֪ͨ���й۲���, priority ԽСԽ���յ�֪ͨ.
  int AddObserver(int priority, ObserverFn&& fn);
  void RemoveObserver(int id);
  // �������� Observer ������������.
  void Notify();

private:
  std::map<std::string, DataType> variables;
  std::map<std::string, ColumnInfo> columns;
  DataCols raw;
  int rowCount;
  int oldRowCount;

  int observerCounter;
  std::map<int, ObserverFn> observers;
};

/* DrawData ---------------------------------------------------------------------*/
// @brief �ڻ�ͼʱ������������Ҫ������.
struct DrawData {
public:
  // ��ͼ�����С
  Size size;
  // ����ͼ��ռ��
  Scalar sWidth;
  // ����ͼ�εı߾�
  Scalar sMargin;

  float wRatio;
  float hRatio;
  DataType bias;

  DrawData(DataSet& data, int off, int count)
    : data(data), off(off), count(count) {
    size = {};
    wRatio = 0.0;
    hRatio = 0.0;
    bias = 0;
  }

  inline int Count() const { return count; }

  inline DataType Get(int col, int row) const {
    return data.Get(col, off + row);
  }
  inline DataType Get(ColumnKey col, int row) const {
    return data.Get(col, off + row);
  }
  inline DataType Get(const Index2& idx) const {
    return data.Get(idx.col, off + idx.row);
  }
  inline bool Empty() const {
    return count == 0;
  }
  inline int NativeIdx(int idx) const {
    return idx - off;
  }

  // @brief ��ȡ���ݵ�X��λ��, ���ڵ���ͼ�����Լ���ռ��,
  // ����Ϊ�˷����ͼ, ���ؾ���λ��.
  inline Scalar ToPX(int idx) const {
    if (sWidth == 1) {
      return Scalar(wRatio * float(idx));
    }

    if ((size.width / sWidth) > count)
      return idx * sWidth + (sWidth / 2 + 1);

    return size.width - (count - idx) * sWidth + (sWidth / 2 + 1);
  }

  // @brief ��ȡ���ݵ�Y��λ��
  inline Scalar ToPY(DataType val) const {
    return size.height - Scalar(hRatio * float(val - bias));
  }

  // @brief ������Ļ�������ö�Ӧ��������
  inline int ToIdx(Scalar px) const {
    int index;
    int last = count - 1;
    if (sWidth == 1) {
      index = int(floor(float(px) / wRatio));
    }
    else {
      // һ����˵, else �ķ������ܵõ�����, ��Ҫ���ǵ�ͼ��δ
      // ��������ʱ�Ǵ���߿�ʼ��ͼ��.
      if ((size.width / sWidth) > count) {
        index = px / sWidth;
      }
      else {
        index = ((size.width - px) / sWidth);
        index = last - index;
      }
    }
    if (index < 0)
      return 0;
    if (index > last)
      return last;
    return index;
  }

  // @brief ������Ļ�������������������
  inline DataType ToData(Scalar py) const {
    // py = size.height - Scalar(hRatio * float(val - bias));
    return DataType(float(size.height - py) / hRatio + float(bias));
  }

private:
  DataSet& data;
  int off;
  int count;
};

/* CrosshairDelegate ---------------------------------------------------------------*/
class CrosshairDelegate {
public:
  CrosshairDelegate();
  ~CrosshairDelegate() = default;

  Point GetCrosshairPoint() const;
  int GetCrosshairIndex() const;

  void SetForceChange();

  virtual void OnMoveCrosshair(Point point);
  // ��������ϵͳʹ�õ����ͣ���ᵼ����ͼ�α���Сʱͨ�� Point
  // �޷�׼ȷ��λ��ͼ������, �����ṩһ������ֱ�����������Ľӿ�.
  virtual void OnMoveCrosshair(int idx);

  virtual void OnPrePaint(GraphContext* ctx, DrawData& data);

  virtual void OnCrosshairIdxChanged(GraphContext* ctx, DrawData& data) { }

protected:
  // ��������-1ʱ,ʮ�ֹ��Ķ�λ���Դ�Ϊ׼, ��������ֵ��߼�����Ϊ��
  // ���������: 10 != data.ToPX(data.ToIdx(10))
  int mstrCrosshairIdx_;

  bool forceChange_;

  Point crosshairPoint_;
  int crosshairIndex_;
};

/* VerticalAxis ----------------------------------------------------------------------*/
class VerticalAxis {
public:
  explicit VerticalAxis(Scalar width, bool aRight)
    : width_(width)
    , scaleColor_(0xFF8F8F8F)
    , alignToRight_(aRight)
    , precision_(1)
    , decimals_(2)
    , transformFn_(DataToStr)
    , crosshairY_(-1)
    , crosshairBackColor_(0xFF3F3F3F) {
  }

  virtual ~VerticalAxis() = default;

  /**
   * ������Լ���������ϵ�����Ӧ����ʲô���ģ�Ĭ���ǽ�����
   * ����2λС��.
   * @param fn
   */
  void SetFormatter(TransformFn fn) { transformFn_ = fn; }
  void SetPrecision(int n) { precision_ = n; }
  void SetDecimals(int d) { decimals_ = d; }
  void SetScaleColor(Color color);
  void SetCrosshairBackColor(Color color);

  Scalar GetWidth() const { return width_; }
  void SetWidth(Scalar width) { width_ = width; }

  virtual void OnSetScales(const DataRows& scales);
  virtual void OnMoveCrosshair(Point point);
  virtual Scalar OnMeasureWidth(GraphContext* ctx);
  virtual void OnPaint(
    GraphContext* ctx,
    DrawData& data,
    Scalar offY
  );

private:
  // ����ͨ�������ж���������߻����ұ�.
  bool alignToRight_;

  Scalar width_;
  Color scaleColor_;
  Scalar crosshairY_;
  CStringW crosshairText_;
  // ʮ�ֳ߿̶����ֵĵ�ɫ.
  Color crosshairBackColor_;

  int precision_;
  int decimals_;
  TransformFn transformFn_;

  std::vector<DataType> scales_;
  // ���ƿ̶�ʱ��ʹ�õĿɲ����������ͣ�Ϊ�˱����ظ�ת�������ǻ���һ��.
  std::vector<CStringW> strScales_;
  std::vector<Size> scaleSize_;
};

/* HorizontalAxis -------------------------------------------------------*/
class KChartWnd;

/// ����һ������̶���
class HorizontalAxis : public CrosshairDelegate {
public:
  explicit HorizontalAxis(KChartWnd* wnd);

  //  void SetScaleFormatter(TransformFn fn) {
  //    transformFn_ = fn;
  //  }
  Scalar GetHeight() const {
    return height_;
  }
  /// @brief ���ÿ̶�ֵ��ʹ�õ���.
  void SetScaleCKey(ColumnKey ckey) {
    hdKey_ = ckey;
  }
  void SetScaleColor(Color color);
  void SetCrosshairBackColor(Color color);

  void OnCrosshairIdxChanged(GraphContext* ctx, DrawData& data) override;

  virtual void OnFitIdx(
    int begin, int end
  );
  virtual void OnPaint(
    GraphContext* ctx,
    DrawData& data,
    Scalar width
  );

private:
  KChartWnd* master_;
  // ������ʲô��Ϊ�̶�ֵ.
  ColumnKey hdKey_;

  Scalar height_;
  Color scaleColor_;
  Scalar crosshairX_;
  CStringW crosshairText_;
  Color crosshairBackColor_;

  //  TransformFn transformFn_;

  std::vector<int> scales_;
  std::vector<CStringW> strScales_;
};

/* Graphics ---------------------------------------------------------------*/
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
  virtual Color GetColor(const DrawData& data, int i) {
    return NormalColor;
  }

  /// @brief ��ȡͼ����ɫ, ��������������.
  virtual Color GetColorWithCA(const DrawData& data, int i) {
    return NormalColor;
  }

  /// ��д Paint �Ի�����Ҫ��ͼ��
  virtual void Paint(
    GraphContext* gctx,
    const DrawData& data
  ) = 0;

  std::vector<ColumnKey> cids;
  DataType centralAxis;

  bool  ZeroOrigin;
  int   Digit;
  Color NormalColor;
};

/// ���ͼ��,
class CombGraph : public Graphics {
public:
  CombGraph() : Graphics(0) { }

public:
  std::vector<Graphics*> graph_;
};

/// K��ͼ����Ҫ�ṩ���ߵ���4������
/// �����ֱ���: OPEN, HIGH, LOW, CLOSE
class KLineGraph : public Graphics {

  enum Cid { Open = 0, High, Low, Close, };

public:
  Color UpColor;
  Color DownColor;
  Color TextColor;

  explicit KLineGraph(DataSet& data)
    : Graphics(4) {
    UpColor = 0xFFFF4A4A;
    DownColor = 0xFF54FCFC;
    TextColor = NormalColor;

    cids[Open] = data.FindCol("OPEN");
    cids[High] = data.FindCol("HIGH");
    cids[Low] = data.FindCol("LOW");
    cids[Close] = data.FindCol("CLOSE");
  }

  void ChangeTheme(bool white) override {
    Graphics::ChangeTheme(white);
    TextColor = NormalColor;
    DownColor = white ? 0xFF10AB62 : 0xFF54FCFC;
  }

  void Paint(
    GraphContext* gctx,
    const DrawData& data
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
    GraphContext* gctx,
    const DrawData& data
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

  Color GetColorWithCA(const DrawData& data, int i) override {
    DataType val = data.Get(cids[0], i);

    if (val > centralAxis)
      return UpColor;
    else if (val < centralAxis)
      return DownColor;

    return NormalColor;
  }

  void Paint(
    GraphContext* gctx,
    const DrawData& data
  ) override;
};

/// @brief ������״ͼ, ��Ҫ����2������(OPEN,CLOSE)������������
/// ͼ��ɫ, ����K��ͼ�еĳɽ���(��).
class VolumeGraph : public HistogramGraph {
public:
  explicit VolumeGraph(DataSet& data)
    : HistogramGraph(data.FindCol("VOLUME")) {
    openKey_ = data.FindCol("OPEN");
    closeKey_ = data.FindCol("CLOSE");
  }

  Color GetColor(const DrawData& data, int i) override {
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

/* GraphArea ---------------------------------------------------------*/
/**
 * ͼ�����������²��ֵķ�ʽչʾ�� KCharWnd ��.
 */
class GraphArea : public CrosshairDelegate {
public:
  virtual ~GraphArea();

  virtual VerticalAxis* GetLeftAxis() {
    return lAxis_;
  }
  virtual VerticalAxis* GetRightAxis() {
    return rAxis_;
  }

  DataType GetCentralAxis();
  /**
   * ����һ��������
   * @param n NAN ��ʾȡ������.
   */
  void SetCentralAxis(DataType n);
  /**
   * �Ƿ���Ҫǿ���������չʾ.
   * @param flag
   */
  void SetBoldCentralAxis(bool flag);
  void SetZeroOrigin(bool flag);
  void SetDecimals(int decimals);
  void SetLabelVisible(bool flag);
  void SetLabelBackColor(Color color);
  void SetScaleLineColor(Color color);

  bool AddGraphics(Graphics* graph);
  bool AddGraphics(const std::vector<Graphics*>& graph);

protected:
  explicit GraphArea(KChartWnd* panel,
    VerticalAxis* la,
    VerticalAxis* ra);

  void SetWeight(float weight);
  float GetWeight() const;
  void SetBounds(const Rect& bounds);
  Rect GetBounds() const;
  Scalar GetLabelHeight() const;
  // ����ǰ����չʾ���������ݵ� min max
  DataType GetMin() const;
  DataType GetMax() const;
  const DataRows& GetScales();

  Scalar GetContentTop() const;
  Scalar GetContentHeight() const;
  void ReGatherLabel(GraphContext* ctx, DrawData& data);

  virtual void UpdateMinMax();
  virtual void UpdateScales();

  void OnCrosshairIdxChanged(GraphContext* ctx, DrawData& data) override;

  virtual void OnFitIdx(int begin, int end);
  virtual void OnPaint(GraphContext* ctx, DrawData& data);
  virtual void OnPaintLabel(GraphContext* ctx, DrawData& data);
  virtual void OnPaintGraph(GraphContext* ctx, DrawData& data);
  virtual void OnPaintCrosshair(GraphContext* ctx, DrawData& data);

protected:
  KChartWnd* panel_;
  float weight_;

  Rect bounds_;
  Rect graphArea_;

  Scalar labelHeight_;
  bool labelVisible_;
  Color labelBackColor_;

  /// ʮ��������λ�����ݿ���ͨ����ǩչʾ.
  struct Label {
    CStringW text;
    Color color;
    Size size;
  };
  std::vector<Label> labels_;

  int begin_;
  int end_;
  int cacheMinIdx_;
  DataType cacheMin_;
  int cacheMaxIdx_;
  DataType cacheMax_;
  bool validCache_;
  int decimals_;

  Color scaleLineColor_;
  int colorIdx_;
  // ������ɫ�б�, ��ͼ��ʹ��.
  std::vector<Color> colorList_;
  DataRows scales_;
  VerticalAxis* lAxis_;
  VerticalAxis* rAxis_;
  DataType centralAxis_;
  bool boldCA_;
  bool zeroOrigin_;

  std::vector<Graphics*> graphics_;
  std::vector<ColumnKey>  columns_;

  friend KChartWnd;
  friend VerticalAxis;
};

class WndProcThunk;

/* KChartWnd ------------------------------------------------------------------*/
/// @brief ����ͼ��Ĵ�����, ����Ϊ������������, Ҳ������
/// �����е�һ���ؼ�.
class KChartWnd {
public:
  KChartWnd();
  /**
   * @brief ����һ��ͼ�괰�壬��Ҫ�ṩһ�����ݱ�
   * ͼ�����ݽ�ͨ�������ݱ��ȡ�����߿��Ը������
   * ����ʹ���޲ι��� KChartWnd()�����ڲ����д�����
   * @param data ͼ�����ݱ�����Ϊ null.
   */
  explicit KChartWnd(std::shared_ptr<DataSet> data);
  virtual ~KChartWnd();

  HWND Handle() {
    return handle_;
  }
  DataSet& DataRef() {
    return *data_;
  }
  void SetCrosshairColor(Color color) {
    crosshairColor_ = color;
  }
  Color GetCrosshairColor() const {
    return crosshairColor_;
  }
  GraphContext* GetGraphContext() {
    return gcContext_;
  }

  /// @brief ������õĺ���, ʹ��WindowsAPI��������
  /// hParent ��������ľ��, ��Ϊ��.
  bool CreateWin(HWND hParent = nullptr);

  Scalar Width() const;
  Scalar Height() const;
  Size GetSize() const;
  Point GetLocation() const;
  Rect GetBounds() const;
  std::string GetTitle() const;
  void SetSize(const Size& size);
  void SetLocation(const Point& point);
  void SetBounds(const Rect& rect);
  void SetTitle(const std::string& str);

  void Show(bool show);
  /// @brief ʹ��������ʧЧ, �Ӷ������ػ��¼�
  /// ��ʹ���淢���˱仯, ����Ҫ���ô˺��������ػ�.
  void Invalidate();

  /// @brief �л�����: �ռ��ҹ��
  void ChangeTheme(bool white);

  Rect GetAreaBounds();

  /**
   * @brief ����ͼ������, ���Դ������(���½ṹ),
   * ÿ�������Ƕ���������ϵ, percent ����ָ����
   * ��Ȩ��(�ڲ�����Ȩ�ط���߶�).
   */
  GraphArea* CreateArea(float percent);
  /**
   * ���ú���
   * @param axis �ṩһ��������д�ĺ���ʵ��.
   * @return
   */
  void SetHAxis(HorizontalAxis* axis);
  HorizontalAxis* GetHAxis();
  /**
   * @brief ���ý�����չʾ�����������̶�
   * ���ú��޷�ʹ�� Zoom �� MoveIdx
   * @param count �����㼴�̶��������෴��ȡ���̶�
   */
  virtual void SetFixedCount(int count);
  /// @brief ��ͼ�ν�������.
  /// factor �����Ŵ�, ������С.
  virtual void Zoom(int factor);
  /// @brief ���������ƶ�ʮ����.
  virtual void MoveCrosshair(int offset);
  virtual void FastScroll(int dir);

public:
  static std::string ClassName() {
    return "WINEX_CLS_KChart";
  }

protected:
  virtual void OnSetCrosshairPoint(Point point);

  LRESULT CALLBACK OnMessage(
    UINT msg, WPARAM wParam, LPARAM lParam
  );

  virtual LRESULT OnProcCreate();
  virtual LRESULT OnProcSize(Scalar width, Scalar height);
  virtual LRESULT OnProcPaint(Rect rect);
  virtual LRESULT OnProcLBtnDown(Point point);
  virtual LRESULT OnProcMouseMove(Point point);

private:
  void FillDrawData(GraphArea* area, DrawData& data);
  void Layout();
  void FitNewWidth();

private:
  HWND handle_;

  // ���浱ǰ��������, ͨ���۲����ݼ��еı仯
  // �ж��Ƿ�����������.
  int rowCount_;

  Color borderColor_;
  Color backColor_;
  Size size_;

  // �Ƿ���ʾ��������(������������).
  bool lvVisible_;
  bool rvVisible_;
  Scalar vAxisWidth_;
  std::vector<VerticalAxis*> lvAxis_;
  std::vector<VerticalAxis*> rvAxis_;
  std::vector<GraphArea*> areas_;
  HorizontalAxis* hAxis_;

  bool crosshairEnable_;
  bool crosshairVisible_;
  Color crosshairColor_;
  Point crosshairPoint_;

  // �����ܹ���ʾ�����ݷ�Χ, �����Χ�ǿ��Գ��� DataSet
  // ���ִ������, ���������ž���ͼ��������ұ߻����
  // һƬ�հ�.
  int  beginIdx_;
  int  endIdx_;
  bool fixedCount_;
  // ����ͼ����ռ���, ��ͨ���ǵ���, ֻ����ͼ�α�������
  // С��3������ʱ,�Ż����˫��, ������ʱӦ�ý������Ϊ
  // ͼ��֮��ľ��������, ��ͼ�εĿ�Ȼᱻ��Ϊ1
  float sWidth_;

  std::shared_ptr<DataSet> data_;

  WndProcThunk* procThunk_;
  GraphContext* gcContext_;

};

}

#endif // LIBKCHART_H
