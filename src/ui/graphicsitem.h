#pragma once

#include <QGraphicsObject>
#include <memory>

namespace ui {

struct gvLayout;

struct GraphicsItem : QGraphicsObject {
  GraphicsItem(int w, int h, QGraphicsItem* parent = nullptr);

  void setSize(int w, int h);
  void setMinimumSize(int w, int h);
  void setPreferredSize(int w, int h);
  void setLayout(std::unique_ptr<gvLayout> layout);

  gvLayout* ownLayout() const {
    return layout_.get();
  }

  QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
             QWidget* widget = nullptr) override;

private:
  friend struct gvLayout;
  friend struct gvRowLayout;

  gvLayout* parent_ = nullptr;
  std::unique_ptr<gvLayout> layout_;

  int width_, height_;
  int w_, h_;
  int minw_, minh_;
};

} // namespace ui