#include <gvlayout.h>
#include <graphicsitem.h>
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>

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
    }
  }
};

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  QGraphicsScene scene;
  MyView view;
  view.setScene(&scene);
  auto m = new ui::GraphicsItem(500, 500);
  scene.addItem(m);
  scene.setItemIndexMethod(QGraphicsScene::NoIndex);

  auto layout = new ui::gvRowLayout;
  for (int i = 0; i < 10; ++i) {
    auto* w = new ui::GraphicsItem(20, 20);
    layout->addWidget(w);
  }
  m->setLayout(std::unique_ptr<ui::gvLayout>(layout));
  view.rectItem = m;
  view.show();
  return app.exec();
}