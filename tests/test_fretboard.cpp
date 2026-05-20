#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include "GuitarFretboardWidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QWidget window;
    QVBoxLayout* layout = new QVBoxLayout(&window);
    GuitarFretboardWidget* fretboard = new GuitarFretboardWidget;
    // Example: E major chord
    GuitarChordShape eMajor;
    eMajor.frets = {0, 2, 2, 1, 0, 0};
    eMajor.name = "E";
    fretboard->setChord(eMajor);
    layout->addWidget(fretboard);
    window.setWindowTitle("Guitar Fretboard Test");
    window.show();
    return app.exec();
}
