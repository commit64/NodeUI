#include <gvlayout.h>
#include <graphicsitem.h>
#include <QTimer>
#include <algorithm>

namespace ui {

gvLayout::~gvLayout() {
  for (const auto item : items_) {
    if (item->type_ == ItemType::Widget) {
      delete static_cast<gvWidget*>(item);
    } else {
      delete static_cast<gvLayout*>(item);
    }
  }
}

void gvLayout::addWidget(GraphicsItem* widget) {
  widget->parent_ = this;
  items_.push_back(new gvWidget(widget));
}

void gvLayout::addLayout(gvLayout* layout) {
  layout->root_ = this;
  items_.push_back(layout);
}

void gvLayout::getItemSize(gvItem* item, int& w, int& h) {
  if (item->type_ == ItemType::Widget) {
    auto widget = static_cast<gvWidget*>(item);
    w = widget->parent_->w_;
    h = widget->parent_->h_;
  } else {
    auto layout = static_cast<gvLayout*>(item);
    w = layout->w_;
    h = layout->h_;
  }
}

void gvLayout::getItemMinSize(gvItem* item, int& w, int& h) {
  if (item->type_ == gvItem::ItemType::Widget) {
    auto widget = static_cast<gvWidget*>(item);
    w = widget->parent_->minw_;
    h = widget->parent_->minh_;
  } else {
    auto layout = static_cast<gvLayout*>(item);
    w = layout->minw_;
    h = layout->minh_;
  }
}

int gvLayout::fitItemSize(const int item, const int pos, const int all,
                          const int available, const bool is_last,
                          float& extra) {
  if (is_last) {
    return available - pos;
  }
  float width = item * available / (float)all + extra;
  extra = width - (int)width;
  return (int)width;
}

void gvLayout::setParentItem(GraphicsItem* parent) {
  for (auto item : items_) {
    if (item->type_ == ItemType::Widget) {
      auto widget = static_cast<gvWidget*>(item)->parent_;
      widget->setParentItem(parent);
    } else {
      auto layout = static_cast<gvLayout*>(item);
      layout->setParentItem(parent);
    }
  }
}

void gvLayout::setupGeom() {
  if (flag_ & ItemFlag::NotDirty) {
    return;
  }

  flag_ |= ItemFlag::NotDirty;

  w_ = 0;
  h_ = 0;
  minw_ = 0;
  minh_ = 0;

  for (const auto item : items_) {
    gvLayout* layout;
    if (item->type_ == ItemType::Widget) {
      layout = static_cast<gvWidget*>(item)->parent_->layout_.get();
    } else {
      layout = static_cast<gvLayout*>(item);
    }
    if (layout) {
      layout->setupGeom();
    }
  }
  setupGeom_impl();
}

void gvLayout::calcGeom() {
  calcGeom_impl();
  for (const auto item : items_) {
    item->flag_ &= ~ItemFlag::AutoFixed;
    gvLayout* layout;
    if (item->type_ == ItemType::Widget) {
      layout = static_cast<gvWidget*>(item)->parent_->layout_.get();
    } else {
      layout = static_cast<gvLayout*>(item);
    }
    if (layout) {
      layout->calcGeom();
    }
  }
}

gvLayout* gvLayout::updateTop() {
  if (!top_ || top_->root_) {
    auto* r = this;
    while (r->root_) {
      r = r->root_;
    }

    auto* p = this;
    do {
      p->top_ = r;
      p = p->root_;
    } while (p && p->top_ != r);
  }

  return top_;
}

void gvLayout::flush() {
  flag_ &= ~ItemFlag::AutoFixed;
  flag_ &= ~ItemFlag::Pending;
  if (flag_ & ItemFlag::SizeHintChanged) {
    flag_ &= ~ItemFlag::SizeHintChanged;
    setupGeom();
  }
  calcGeom();
}

void gvLayout::update(uint8_t flag) {
  auto* root = updateTop();
  if (flag & ItemFlag::SizeHintChanged) {
    auto* p = this;
    do {
      if (!(p->flag_ & ItemFlag::NotDirty)) {
        break;
      }
      p->flag_ &= ~ItemFlag::NotDirty;
      p = p->root_;
    } while (p);
  }

  root->flag_ |= flag;

  if (root->flag_ & ItemFlag::Pending) {
    return;
  }
  root->flag_ |= ItemFlag::Pending;
  QTimer::singleShot(0, root->parent_, [root] { root->flush(); });
}

void gvRowLayout::setupGeom_impl() {
  for (const auto item : items_) {
    int itemw, itemh;
    getItemSize(item, itemw, itemh);
    w_ += itemw;
    h_ = std::max(itemh, h_);
    getItemMinSize(item, itemw, itemh);
    minw_ += itemw;
    minh_ = std::max(itemh, minh_);
  }
}

void gvRowLayout::calcGeom_impl() {
  int n = (int)items_.size();
  int force_minw = minw_ + 2 * margin_ + (n > 0 ? spacing_ * (n - 1) : 0);
  int force_minh = minh_ + 2 * margin_;
  if (!root_) {
    width_ = std::max(parent_->width_, force_minw);
    height_ = std::max(parent_->height_, force_minh);
  }

  int fixedw = 0, new_fixedw = 0;
  int freew = w_, new_freew = w_;

  int last_free_idx = -1;
  int idx;

  bool flag;
  do {
    idx = -1;
    int x = 0;
    float extra = 0;
    flag = false;

    for (const auto item : items_) {
      if (item->flag_ & ItemFlag::AutoFixed) {
        continue;
      }

      bool is_last = item == items_.back();
      int itemw, itemh;
      getItemSize(item, itemw, itemh);
      int item_minw, item_minh;
      getItemMinSize(item, item_minw, item_minh);

      int new_itemw;
      int available = width_ - fixedw - 2 * margin_ - spacing_ * (n - 1);
      if (available > 0) {
        new_itemw = fitItemSize(itemw, x, freew, available, is_last, extra);
      } else {
        new_itemw = 0;
      }

      x += new_itemw;

      if ((new_itemw < item_minw) || (item->flag_ & ItemFlag::Fixed)) {
        item->flag_ |= ItemFlag::AutoFixed;
        new_freew -= itemw;
        new_fixedw += item_minw;
        flag = true;
      }
    }

    fixedw = new_fixedw;
    freew = new_freew;
  } while (flag);

  for (int i = (int)items_.size() - 1; i >= 0; --i) {
    if (!(items_[i]->flag_ & ItemFlag::AutoFixed)) {
      last_free_idx = i;
      break;
    }
  }

  int x = margin_;
  int y;
  int available = width_ - fixedw - 2 * margin_ - spacing_ * (n - 1);
  float extra = 0;
  int used = 0;
  idx = -1;
  for (const auto item : items_) {
    idx += 1;
    bool is_last = idx == last_free_idx;
    int itemw, itemh;
    getItemSize(item, itemw, itemh);
    if (item->flag_ & ItemFlag::AutoFixed) {
      getItemMinSize(item, itemw, itemh);
    } else {
      itemw = fitItemSize(itemw, used, freew, available, is_last, extra);
      used += itemw;
    }

    y = margin_ + (height_ - itemh) / 2;

    int xpos = x + x_;
    int ypos = y + y_;
    if (item->type_ == ItemType::Widget) {
      auto widget = static_cast<gvWidget*>(item);
      auto parent = widget->parent_;
      parent->width_ = itemw;
      parent->height_ = itemh;
      parent->setPos(xpos, ypos);
      if (parent->layout_) {
        auto layout = parent->layout_.get();
        layout->width_ = itemw;
        layout->height_ = itemh;
        layout->x_ = xpos;
        layout->y_ = ypos;
      }
    } else {
      auto layout = static_cast<gvLayout*>(item);
      layout->width_ = itemw;
      layout->height_ = itemh;
      layout->x_ = xpos;
      layout->y_ = ypos;
    }

    x += itemw + spacing_;
  }
}

} // namespace ui