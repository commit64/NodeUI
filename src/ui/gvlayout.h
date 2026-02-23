#pragma once

#include <vector>

namespace ui {

struct GraphicsItem;

struct gvItem {
  enum ItemType : uint8_t { Unknown, Widget, LayoutRow, LayoutColumn };
  enum ItemFlag : uint8_t {
    AutoFixed = 1 << 0,
    Fixed = 1 << 1,
    NotDirty = 1 << 2,
    SizeHintChanged = 1 << 3, // only for top
    OnlyRecalc = 1 << 4,      // only for top
    Pending = 1 << 5          // only for top
  };
  enum LayoutAlignment : uint8_t { Left, Center, Right };

  gvItem(ItemType type)
      : type_(type) {
  }
  // virtual ~gvItem() = default;

  uint8_t type_ = 0;
  uint8_t flag_ = 0;
};

struct gvWidget : gvItem {
  gvWidget(GraphicsItem* parent)
      : gvItem(ItemType::Widget),
        parent_(parent) {
  }

  GraphicsItem* parent_;
};

struct gvLayout : gvItem {
  gvLayout(ItemType type)
      : gvItem(type) {
  }

  virtual ~gvLayout();

  void addWidget(GraphicsItem* widget);

private:
  friend struct GraphicsItem;
  friend struct gvRowLayout;

  gvLayout* updateTop();
  void flush(uint8_t flag);
  void update(uint8_t flag);
  void setParentItem();
  void removeWidget(GraphicsItem* widget);

  void getItemSize(gvItem* item, int& w, int& h);
  void getItemMinSize(gvItem* item, int& w, int& h);
  int fitItemSize(const int item, const int pos, const int all,
                  const int available, const bool is_last, float& extra);

  void setupGeom(); // only for top
  void calcGeom();  // only for top
  virtual void setupGeom_impl() = 0;
  virtual void calcGeom_impl() = 0;

  using gvItem::flag_;
  using gvItem::type_;

  int width_ = 0, height_ = 0;
  int w_ = 0, h_ = 0;
  int minw_ = 0, minh_ = 0;
  int margin_ = 0;
  int spacing_ = 0;
  uint8_t align_ = 0;

  gvLayout* root_ = nullptr;
  gvLayout* top_ = nullptr;
  GraphicsItem* parent_ = nullptr;
  std::vector<gvItem*> items_;
};

struct gvRowLayout final : gvLayout {
  gvRowLayout()
      : gvLayout(ItemType::LayoutRow) {
  }

  using gvLayout::addWidget;
  using gvLayout::removeWidget;

private:
  void setupGeom_impl() override;
  void calcGeom_impl() override;
};

} // namespace ui