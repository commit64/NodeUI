#include <graphicsitem.h>
#include <gvlayout.h>
#include <QPainter>

namespace ui {

GraphicsItem::GraphicsItem(int w, int h, QGraphicsItem* parent)
    : QGraphicsObject(parent) {
  width_ = w, height_ = h;
  w_ = w, h_ = h;
  minw_ = 0, minh_ = 0;
}

void GraphicsItem::setSize(int w, int h) {
  if (!parent_) {
    width_ = std::max(minw_, w);
    height_ = std::max(minh_, h);
  }
}

void GraphicsItem::setPreferredSize(int w, int h) {
  w_ = std::max(w, minw_);
  h_ = std::max(h, minh_);
}

void GraphicsItem::setMinimumSize(int w, int h) {
  minw_ = w;
  minh_ = h;
}

void GraphicsItem::setLayout(std::unique_ptr<gvLayout> layout) {
  layout_ = std::move(layout);
  layout_->parent_ = this;
  layout_->setParentItem(this);
}

QRectF GraphicsItem::boundingRect() const {
  return QRectF(0, 0, width_, height_);
}

void GraphicsItem::paint(QPainter* painter,
                         const QStyleOptionGraphicsItem* option,
                         QWidget* widget) {
  QPen pen(Qt::red);
  painter->setPen(pen);
  painter->drawRect(0, 0, width_, height_);
}

} // namespace ui