#include <gvlayout.h>
#include <graphicsitem.h>
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  QGraphicsScene scene;
  QGraphicsView view(&scene);

  auto w1 = new ui::GraphicsItem(15, 20);
  auto w2 = new ui::GraphicsItem(30, 60);
  auto w3 = new ui::GraphicsItem(1, 80);
  auto w11 = new ui::GraphicsItem(15, 20);
  auto w22 = new ui::GraphicsItem(30, 60);
  auto w33 = new ui::GraphicsItem(1, 80);
  auto layout = std::make_unique<ui::gvRowLayout>();
  layout->addWidget(w1);
  layout->addWidget(w2);
  layout->addWidget(w3);
  ui::GraphicsItem w(200, 200);
  auto m = new ui::GraphicsItem(400, 400);
  w.setLayout(std::move(layout));
  auto root = std::make_unique<ui::gvRowLayout>();
  root->addWidget(&w);
  root->addWidget(w11);
  root->addWidget(w22);
  root->addWidget(w33);
  m->setLayout(std::move(root));

  scene.addItem(m);
  view.show();
  return app.exec();
}