#include <gvlayout.h>
#include <graphicsitem.h>
#include <QTimer>
#include <algorithm>

namespace ui {

gvLayout::~gvLayout() {
  for (const auto item : items_) {
    if (item->type_ == ItemType::Widget) {
      delete static_cast<gvWidget*>(item);
    }
  }
}

void gvLayout::addWidget(GraphicsItem* widget) {
  if (!widget) {
    return;
  }

  if (widget->parent_) {
    removeWidget(widget);
    return;
  }

  widget->parent_ = this;

  if (auto* own_layout = widget->layout_.get()) {
    own_layout->root_ = this;
    items_.push_back(own_layout);
  } else {
    items_.push_back(new gvWidget(widget));
  }

  if (parent_) {
    widget->setParentItem(parent_);
  }
  update(ItemFlag::SizeHintChanged);
}

void gvLayout::removeWidget(GraphicsItem* widget) {
  if (auto* own_layout = widget->layout_.get()) {
    auto it = std::find(items_.cbegin(), items_.cend(), own_layout);
    items_.erase(it);
    own_layout->root_ = nullptr;
    own_layout->top_ = nullptr;
    own_layout->flag_ &= ~ItemFlag::AutoFixed;
  } else {
    auto it = std::find_if(items_.cbegin(), items_.cend(), [&](auto* item) {
      return static_cast<gvWidget*>(item)->parent_ == widget;
    });
    delete static_cast<gvWidget*>(*it);
    items_.erase(it);
  }
  // widget->parent_ = nullptr;
  widget->setParentItem(nullptr);
  update(ItemFlag::SizeHintChanged);
}

void gvLayout::getItemSize(gvItem* item, int& w, int& h) {
  if (item->type_ == gvItem::ItemType::Widget) {
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

void gvLayout::setParentItem() {
  for (auto item : items_) {
    if (item->type_ == ItemType::Widget) {
      auto widget = static_cast<gvWidget*>(item)->parent_;
      widget->setParentItem(parent_);
    } else {
      auto widget = static_cast<gvLayout*>(item)->parent_;
      widget->setParentItem(parent_);
    }
  }
}

void gvLayout::setupGeom() {
  if (!parent_) {
    return;
  }
  if (flag_ & ItemFlag::NotDirty) {
    return;
  }

  flag_ |= ItemFlag::NotDirty;

  w_ = 0;
  h_ = 0;
  minw_ = 0;
  minh_ = 0;
  flag_ &= ~ItemFlag::AutoFixed;

  for (const auto item : items_) {
    if (item->type_ == ItemType::Widget) {
      continue;
    }
    auto layout = static_cast<gvLayout*>(item);
    layout->setupGeom();
  }
  setupGeom_impl();
}

void gvLayout::calcGeom() {
  if (!parent_) {
    return;
  }
  calcGeom_impl();
  for (const auto item : items_) {
    if (item->type_ == ItemType::Widget) {
      continue;
    }
    auto layout = static_cast<gvLayout*>(item);
    layout->calcGeom();
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

void gvLayout::flush(uint8_t flag) {
  flag_ &= ~ItemFlag::Pending;
  if (flag & ItemFlag::SizeHintChanged) {
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

  if (root->flag_ & ItemFlag::Pending) {
    return;
  }
  root->flag_ |= ItemFlag::Pending;
  QTimer::singleShot(0, root->parent_, [root, flag] { root->flush(flag); });
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
  width_ = std::max(parent_->width_, force_minw);
  height_ = std::max(parent_->height_, force_minh);

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
      idx += 1;
      if (item->flag_ & ItemFlag::AutoFixed) {
        continue;
      }

      bool is_last = item == items_.back();
      int itemw, itemh;
      getItemSize(item, itemw, itemh);
      int item_minw, item_minh;
      getItemMinSize(item, item_minw, item_minh);
      x += itemw;

      int new_itemw;
      int available = width_ - fixedw - 2 * margin_ - spacing_ * (n - 1);
      if (available > 0) {
        new_itemw = fitItemSize(itemw, x, freew, available, is_last, extra);
      } else {
        new_itemw = 0;
      }

      if ((new_itemw <= item_minw) || (item->flag_ & ItemFlag::Fixed)) {
        item->flag_ |= ItemFlag::AutoFixed;
        new_freew -= itemw;
        new_fixedw += item_minw;
        flag = true;
      } else {
        last_free_idx = idx;
      }
    }

    fixedw = new_fixedw;
    freew = new_freew;
  } while (flag);

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

    if (item->type_ == ItemType::Widget) {
      auto widget = static_cast<gvWidget*>(item);
      widget->parent_->width_ = itemw;
      widget->parent_->height_ = itemh;
      widget->parent_->setPos(x, y);
    } else {
      auto layout = static_cast<gvLayout*>(item);
      layout->parent_->width_ = itemw;
      layout->parent_->height_ = itemh;
      layout->parent_->setPos(x, y);
    }

    x += itemw + spacing_;
  }
}

} // namespace ui