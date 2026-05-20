#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include "HorizontalFretboardWidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QWidget window;
    QVBoxLayout* layout = new QVBoxLayout(&window);
    HorizontalFretboardWidget* fretboard = new HorizontalFretboardWidget;
    // Example: E major chord, root and two inversions
    std::vector<GuitarChordShape> inversions;
    inversions.push_back({{0,2,2,1,0,0}, {}, "E (open)"});
    inversions.push_back({{7,7,9,9,9,7}, {}, "E (barre)"});
    inversions.push_back({{12,14,14,13,12,12}, {}, "E (high)"});
    fretboard->setChordInversions(inversions);
    layout->addWidget(fretboard);
    window.setWindowTitle("Horizontal Guitar Fretboard Test");
    window.show();
    return app.exec();
}
