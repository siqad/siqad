#include <QtTest/QtTest>

#include "gui/widgets/managers/layer_manager.h"
#include "gui/widgets/primitives/lattice.h"

class SiQADTests: public QObject
{
  Q_OBJECT

// functions in these slots are automatically called
private slots:
  
  // void testLayerManager()
  // {
  //   gui::LayerManager *layman = new gui::LayerManager(nullptr);

  //   // Add default basic layers
  //   prim::Lattice *lat = new prim::Lattice();
  //   prim::Lattice *latr = new prim::Lattice();
  //   layman->addLattice(lat);                        // Design lattice
  //   layman->addLattice(latr, prim::Layer::Result);  // Sim Result lattice
  //   layman->addDBLayer(lat, "Surface");             // DB surface
  //   layman->addLayer("Metal", prim::Layer::Electrode, prim::Layer::Design, 1000, 100);  // Metal layer
  //   QCOMPARE(layman->layerCount(), 3);              // Sim Result lattice doesn't count here
  //   QCOMPARE(layman->getLayer(0)->getName(), QString("Lattice"));
  //   QCOMPARE(layman->getLayer(0)->role(), prim::Layer::Design);
  //   QCOMPARE(layman->getLayer(0, false)->getName(), QString("Lattice"));
  //   QCOMPARE(layman->getLayer(0, false)->role(), prim::Layer::Result);
  //   QCOMPARE(layman->getLayer(1)->getName(), QString("Surface"));
  //   QCOMPARE(layman->getLayer(1)->role(), prim::Layer::Design);
  //   QCOMPARE(layman->getLayer(2)->getName(), QString("Metal"));
  //   QCOMPARE(layman->getLayer(2)->role(), prim::Layer::Design);

  //   // Add layer with conflicting name, which should be refused
  //   prim::Layer *lay = layman->addLayer("Metal", prim::Layer::Electrode, prim::Layer::Design, 1, 10);
  //   QCOMPARE(lay, nullptr);
  //   QCOMPARE(layman->layerCount(), 3);
  //   QCOMPARE(layman->getLayer("Metal")->zOffset(), (float) 1000);
  //   QCOMPARE(layman->getLayer("Metal")->zHeight(), (float) 100);

  //   // Add layer and check that the indices between LayerManager and the Layer do match
  //   lay = layman->addLayer("Metal 2", prim::Layer::Electrode, prim::Layer::Design, 1, 10);
  //   QVERIFY(lay != nullptr);
  //   QCOMPARE(lay->layerID(), layman->indexOf(lay));

  //   // TODO add checks for setActiveLayer and getMRULayer

  //   // Remove all layers and check count
  //   layman->removeAllLayers();
  //   QCOMPARE(layman->layerCount(), 0);
  // }

};

QTEST_MAIN(SiQADTests)
#include "siqad_tests.moc"  // generated at compile time

