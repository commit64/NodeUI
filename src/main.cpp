#include <gvlayout.h>
#include <graphicsitem.h>
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QElapsedTimer>
#include <QTimer>

class MyView : public QGraphicsView {
public:
  ui::GraphicsItem* rectItem = nullptr;

protected:
  void resizeEvent(QResizeEvent* e) override {
    QGraphicsView::resizeEvent(e);

    QRectF r(0, 0, viewport()->width(), viewport()->height());
    scene()->setSceneRect(r);

    if (rectItem) {
      rectItem->setSize(r.width(), r.height());
      rectItem->setPos(0, 0);
      rectItem->ownLayout()->updateOnly();
    }
  }

  void paintEvent(QPaintEvent* e) override {
    QElapsedTimer t;
    t.start();
    QGraphicsView::paintEvent(e);
    qDebug() << "paintEvent ms =" << t.elapsed();
  }
};

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  static QElapsedTimer t;
  t.start();
  static qint64 last = t.elapsed();

  auto probe = new QTimer(qApp);
  probe->setInterval(16);
  QObject::connect(probe, &QTimer::timeout, [] {
    auto now = t.elapsed();
    if (last != 0 && now - last > 200) {
      qDebug() << "GUI stall:" << (now - last) << "ms";
    }
    last = now;
  });
  probe->start();
  QGraphicsScene scene;
  MyView view;
  view.setScene(&scene);
  auto m = new ui::GraphicsItem(500, 500);
  scene.addItem(m);
  scene.setItemIndexMethod(QGraphicsScene::NoIndex);
  view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  auto layout = new ui::gvRowLayout;
  for (int i = 0; i < 10; ++i) {
    auto* w = new ui::GraphicsItem(i, i);
    w->setMinimumSize(i, i);
    layout->addWidget(w);
  }
  auto l2 = new ui::gvRowLayout;
  for (int i = 0; i < 30; ++i) {
    auto* w = new ui::GraphicsItem(i, i);
    l2->addWidget(w);
  }
  auto l3 = new ui::gvRowLayout;
  for (int i = 0; i < 30; ++i) {
    auto* w = new ui::GraphicsItem(i, i);
    l3->addWidget(w);
  }

  auto* h = new ui::GraphicsItem(300, 300);
  h->setLayout(std::unique_ptr<ui::gvLayout>(l3));
  layout->addLayout(l2);
  layout->addWidget(h);
  m->setLayout(std::unique_ptr<ui::gvLayout>(layout));
  m->ownLayout()->updateAll();
  view.rectItem = m;
  view.show();
  return app.exec();
}