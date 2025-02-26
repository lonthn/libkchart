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

// 通常来讲，用于展示的数据不可能达到 int64 的最大值
// 所以我们用它来表示无意义的数据.
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


// 如果将数据完整展示，我们可能没有足够的空间，所以可以
// 尝试带上单位.
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

// 用于衡量图形各维的参数
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
  // 控制原点位置.
  virtual Point Translate(const Point& point) { return { 0, 0 }; }
  virtual void SetTranslate(const Point& point) {}

  virtual void SetFont(FontId fontId, int size) {}
  virtual void SetColor(Color color) {}
  virtual void SetLineWidth(int width) {}

  /**
   * 渲染矩形, DrawRect 代表空心的, FillRect 代表实心.
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
   * 提供两个点, 画一条线段.
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
   * 画一条自定义折线,
   * 提供至少2个或以上的点，将这些点连接完成折线图
   * @param pointNum 指定点的数量.
   * @example
   * // 使用时需要用到3个函数，在调用EndPolyLine时
   * 将所有点连接渲染在画布上:
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
   * 将字符串 str 渲染在画布上
   * @param str
   * @param p 文本左上角的位置.
   */
  virtual void DrawStr(const CStringW& str, const Point& p) {}
  // 要绘制文本, 通常可能需要 测量文本的宽高，
  // 我们会考虑 字体 的 size 参数.
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

// 数据列相关信息
typedef struct ColumnInfo {
  CStringW name;
  // 数据使用整型存储，可以通过它来控制
  // 存储浮点数.
  int precision;
  int index;

  Setter* setter;
} *ColumnKey;

struct Index2 {
  int col;
  int row;
};

// @brief 原始数据集, 以2维表格的形式存储图形对应的原始
// 数据且可随时扩容.
// @details 通常我们会创建一个图形或者自定义图形, 并提
// 供根据图形所需的数据对应的 ColumnKey(通过AddCol/FindCol获得).
struct DataSet {

  typedef std::function<void(DataSet& data, int)> ObserverFn;

  DataSet();

  // @brief 添加列, 需提供列名.
  ColumnKey CreateCol(const std::string& name);
  ColumnKey CreateCol(const std::string& name, int precision);
  ColumnKey CreateCol(const std::string& name, Setter* setter);
  ColumnKey CreateCol(const std::string& name, int precision, Setter* setter);
  // @brief 添加一行.
  int AddRow();
  // @brief 添加行, 可指定行数.
  void AddRow(int n);

  // @brief 根据列名查找 ColumnKey(类索引, 用于获取
  // 对应列的数据).
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
  // @note 通过该函数获得的列数据如果对其进行写入并
  // 不会触发观察者.
  DataRows& operator[](int col);

  void FillRow(void* src, int idx) {
    for (auto& item : columns) {
      ColumnKey col = &item.second;
      if (col->setter)
        (*col->setter)(&(raw[col->index][idx]), src);
    }
  }

  // @brief 添加数据监听, 当Notify被调用后会通过ObserverFn
  // 通知所有观察者, priority 越小越先收到通知.
  int AddObserver(int priority, ObserverFn&& fn);
  void RemoveObserver(int id);
  // 告诉所有 Observer 有新增行数据.
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
// @brief 在绘图时，这里有你需要的数据.
struct DrawData {
public:
  // 绘图区域大小
  Size size;
  // 单个图形占宽
  Scalar sWidth;
  // 单个图形的边距
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

  // @brief 获取数据的X轴位置, 由于单个图形有自己的占宽,
  // 所以为了方便绘图, 返回居中位置.
  inline Scalar ToPX(int idx) const {
    if (sWidth == 1) {
      return Scalar(wRatio * float(idx));
    }

    if ((size.width / sWidth) > count)
      return idx * sWidth + (sWidth / 2 + 1);

    return size.width - (count - idx) * sWidth + (sWidth / 2 + 1);
  }

  // @brief 获取数据的Y轴位置
  inline Scalar ToPY(DataType val) const {
    return size.height - Scalar(hRatio * float(val - bias));
  }

  // @brief 根据屏幕横坐标获得对应数据索引
  inline int ToIdx(Scalar px) const {
    int index;
    int last = count - 1;
    if (sWidth == 1) {
      index = int(floor(float(px) / wRatio));
    }
    else {
      // 一般来说, else 的方案就能得到索引, 但要考虑当图形未
      // 充满界面时是从左边开始绘图的.
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

  // @brief 根据屏幕纵坐标轴算出具体数据
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
  // 由于坐标系统使用的整型，这会导致在图形被缩小时通过 Point
  // 无法准确定位到图形索引, 所以提供一个可以直接设置索引的接口.
  virtual void OnMoveCrosshair(int idx);

  virtual void OnPrePaint(GraphContext* ctx, DrawData& data);

  virtual void OnCrosshairIdxChanged(GraphContext* ctx, DrawData& data) { }

protected:
  // 在他大于-1时,十字光标的定位就以此为准, 有这种奇怪的逻辑是因为存
  // 在这种情况: 10 != data.ToPX(data.ToIdx(10))
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
   * 你可以自己定义界面上的数据应该是什么样的，默认是将数据
   * 保留2位小数.
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
  // 可以通过它来判断我是在左边还是右边.
  bool alignToRight_;

  Scalar width_;
  Color scaleColor_;
  Scalar crosshairY_;
  CStringW crosshairText_;
  // 十字尺刻度文字的底色.
  Color crosshairBackColor_;

  int precision_;
  int decimals_;
  TransformFn transformFn_;

  std::vector<DataType> scales_;
  // 绘制刻度时所使用的可不是数字类型，为了避免重复转换，我们缓存一下.
  std::vector<CStringW> strScales_;
  std::vector<Size> scaleSize_;
};

/* HorizontalAxis -------------------------------------------------------*/
class KChartWnd;

/// 我是一个横向刻度轴
class HorizontalAxis : public CrosshairDelegate {
public:
  explicit HorizontalAxis(KChartWnd* wnd);

  //  void SetScaleFormatter(TransformFn fn) {
  //    transformFn_ = fn;
  //  }
  Scalar GetHeight() const {
    return height_;
  }
  /// @brief 设置刻度值所使用的列.
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
  // 横轴以什么作为刻度值.
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

  /// 在图形被加入GraphArea后，GraphArea会在ColorList中
  /// 选取一个颜色进行设置, 当然我们也可以自定义颜色.
  virtual bool SetColor(Color color) {
    return false;
  }

  /// @brief 获取图形颜色.
  virtual Color GetColor(const DrawData& data, int i) {
    return NormalColor;
  }

  /// @brief 获取图形颜色, 带中心轴的情况下.
  virtual Color GetColorWithCA(const DrawData& data, int i) {
    return NormalColor;
  }

  /// 重写 Paint 以绘制需要的图形
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

/// 组合图形,
class CombGraph : public Graphics {
public:
  CombGraph() : Graphics(0) { }

public:
  std::vector<Graphics*> graph_;
};

/// K线图，需要提供开高低收4列数据
/// 列名分别是: OPEN, HIGH, LOW, CLOSE
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

  // 不需要切换
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

/// 柱状图
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

/// @brief 特殊柱状图, 需要额外2列数据(OPEN,CLOSE)来决定单根柱
/// 图颜色, 用于K线图中的成交量(额).
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
 * 图形区域，以上下布局的方式展示在 KCharWnd 中.
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
   * 设置一个中心轴
   * @param n NAN 表示取消设置.
   */
  void SetCentralAxis(DataType n);
  /**
   * 是否需要强调中心轴的展示.
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
  // 代表当前界面展示的所有数据的 min max
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

  /// 十字线所定位的数据可以通过标签展示.
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
  // 内置颜色列表, 供图形使用.
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
/// @brief 容纳图表的窗体类, 可作为独立窗口运行, 也可以是
/// 窗体中的一个控件.
class KChartWnd {
public:
  KChartWnd();
  /**
   * @brief 构造一个图标窗体，需要提供一个数据表
   * 图形数据将通过该数据表获取，或者可以根据情况
   * 考虑使用无参构造 KChartWnd()，由内部自行创建。
   * @param data 图形数据表，不能为 null.
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

  /// @brief 必须调用的函数, 使用WindowsAPI创建窗体
  /// hParent 即父窗体的句柄, 可为空.
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
  /// @brief 使界面内容失效, 从而触发重绘事件
  /// 若使界面发生了变化, 必须要调用此函数进行重绘.
  void Invalidate();

  /// @brief 切换主题: 日间或夜间
  void ChangeTheme(bool white);

  Rect GetAreaBounds();

  /**
   * @brief 创建图形区域, 可以创建多个(上下结构),
   * 每个区域都是独立的坐标系, percent 可以指定高
   * 度权重(内部根据权重分配高度).
   */
  GraphArea* CreateArea(float percent);
  /**
   * 设置横轴
   * @param axis 提供一个经过重写的横轴实例.
   * @return
   */
  void SetHAxis(HorizontalAxis* axis);
  HorizontalAxis* GetHAxis();
  /**
   * @brief 设置将界面展示的数据条数固定
   * 设置后将无法使用 Zoom 和 MoveIdx
   * @param count 大于零即固定条数，相反则取消固定
   */
  virtual void SetFixedCount(int count);
  /// @brief 对图形进行缩放.
  /// factor 正数放大, 负数缩小.
  virtual void Zoom(int factor);
  /// @brief 横向左右移动十字线.
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

  // 缓存当前数据条数, 通过观察数据集中的变化
  // 判断是否有新增数据.
  int rowCount_;

  Color borderColor_;
  Color backColor_;
  Size size_;

  // 是否显示纵轴坐标(包括左右两边).
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

  // 控制能够显示的数据范围, 这个范围是可以超过 DataSet
  // 中现存的数据, 这种情况大概就是图形区域的右边会出现
  // 一片空白.
  int  beginIdx_;
  int  endIdx_;
  bool fixedCount_;
  // 单个图形所占宽度, 它通常是单数, 只有在图形被缩放至
  // 小于3个像素时,才会出现双数, 但在这时应该将其理解为
  // 图形之间的距离更合适, 而图形的宽度会被置为1
  float sWidth_;

  std::shared_ptr<DataSet> data_;

  WndProcThunk* procThunk_;
  GraphContext* gcContext_;

};

}

#endif // LIBKCHART_H
