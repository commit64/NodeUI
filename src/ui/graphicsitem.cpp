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
  if (layout_) {
    int minw = layout_->minw_;
    int minh = layout_->minh_;
    if ((w < minw && h < minh) || (w == width_ && h == height_)) {
      return;
    }
    width_ = std::max(minw, w);
    height_ = std::max(minh, h);
    update();
    layout_->update(gvItem::ItemFlag::OnlyRecalc);
  } else if (!parent_) {
    if ((w < minw_ && h < minh_) || (w == width_ && h == height_)) {
      return;
    }
    width_ = std::max(minw_, w);
    height_ = std::max(minh_, h);
    update();
  }
}

void GraphicsItem::setPreferredSize(int w, int h) {
  if (w <= minw_ && h <= minh_) {
    return;
  }
  w_ = std::max(w, minw_);
  h_ = std::max(h, minh_);
  if (parent_) {
    parent_->update(gvItem::ItemFlag::SizeHintChanged);
  }
}

void GraphicsItem::setMinimumSize(int w, int h) {
  if (!layout_) {
    return;
  }

  minw_ = w;
  minh_ = h;

  if (parent_) {
    if (w == minw_ && h == minh_) {
      return;
    }
    parent_->update(gvItem::ItemFlag::SizeHintChanged);
  }
}

void GraphicsItem::setLayout(std::unique_ptr<gvLayout> layout) {
  layout_ = std::move(layout);
  layout_->parent_ = this;
  layout_->width_ = width_;
  layout_->height_ = height_;

  if (layout_->items_.empty()) {
    return;
  }

  layout_->setParentItem();
  layout_->update(gvItem::ItemFlag::SizeHintChanged);
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