#include "../widgets/NoteMapFretboardWidget.h"
#include "../utils/NoteMapUtils.h"
#include "../utils/ScaleDefinitions.h"
#include "../widgets/MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QStyleFactory>
#include <vector>
#include <QString>
#include <QMap>
// Helper: get note names for a chord (root, 3rd, 5th, 7th)
std::vector<QString> getChordNotes(const QString& chordName) {
    static const char* noteNames[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    static const QMap<QString, int> noteIndices = {
        {"C",0}, {"C#",1}, {"Db",1}, {"D",2}, {"D#",3}, {"Eb",3}, {"E",4}, {"F",5},
        {"F#",6}, {"Gb",6}, {"G",7}, {"G#",8}, {"Ab",8}, {"A",9}, {"A#",10}, {"Bb",10}, {"B",11}
    };

    int rootIdx = -1;
    QString rootStr;
    QString typeStr;

    if (chordName.length() > 1 && (chordName[1] == '#' || chordName[1] == 'b')) {
        rootStr = chordName.left(2);
    } else {
        rootStr = chordName.left(1);
    }
    typeStr = chordName.mid(rootStr.length());
    rootIdx = noteIndices.value(rootStr, 0);

    // Determine intervals based on chord type
    int third = 4; // Major third
    int fifth = 7; // Perfect fifth
    int seventh = -1; // No seventh

    if (typeStr.contains("m7b5")) { third = 3; fifth = 6; seventh = 10; }
    else if (typeStr.contains("dim7")) { third = 3; fifth = 6; seventh = 9; }
    else if (typeStr.contains("m7")) { third = 3; fifth = 7; seventh = 10; }
    else if (typeStr.contains("maj7")) { third = 4; fifth = 7; seventh = 11; }
    else if (typeStr.contains("7#9")) { third = 4; fifth = 7; seventh = 10; } // #9 not shown as a degree
    else if (typeStr.contains("7b9")) { third = 4; fifth = 7; seventh = 10; } // b9 not shown as a degree
    else if (typeStr.contains("7")) { third = 4; fifth = 7; seventh = 10; }
    else if (typeStr.contains("m")) { third = 3; }
    else if (typeStr.contains("dim")) { third = 3; fifth = 6; }
    else if (typeStr.contains("aug")) { fifth = 8; }
    else if (typeStr.contains("sus4")) { third = 5; } // Use 'third' slot for sus note
    else if (typeStr.contains("sus2")) { third = 2; } // Use 'third' slot for sus note
    else if (typeStr.contains("6")) { seventh = 9; } // Use 'seventh' slot for 6th
    else if (typeStr.contains("add9")) { seventh = 2; } // Use 'seventh' slot for 9th

    std::vector<QString> notes;
    notes.push_back(noteNames[rootIdx]);
    if (third != -1) {
        notes.push_back(noteNames[(rootIdx + third) % 12]);
    }
    if (fifth != -1) {
        notes.push_back(noteNames[(rootIdx + fifth) % 12]);
    }
    if (seventh != -1) {
        // Avoid duplicate notes (e.g. for add9 where 9th might be same as root)
        QString seventhNote = noteNames[(rootIdx + seventh) % 12];
        if (std::find(notes.begin(), notes.end(), seventhNote) == notes.end()) {
            notes.push_back(seventhNote);
        }
    }
    return notes;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Apply a modern style and load the custom stylesheet
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QFile styleFile(":/style.qss"); // Assuming style.qss is added to resources
    if (!styleFile.exists()) {
        styleFile.setFileName("style.qss"); // Fallback for running from build dir
    }
    styleFile.open(QFile::ReadOnly | QFile::Text);
    app.setStyleSheet(QLatin1String(styleFile.readAll()));

    MainWindow window;
    window.show();
    return app.exec();
}
