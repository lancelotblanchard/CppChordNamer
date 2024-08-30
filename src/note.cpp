#include <stdexcept>

#include "note.h"

ChordNamer::Note::Note(const std::string &str, const Accidental preferredAccidental): accidental(NATURAL),
    preferredAccidental(preferredAccidental) {
    if (!validate(str)) {
        throw std::invalid_argument("Invalid note: " + str);
    }

    int index = -1;
    for (const char c: str) {
        if (c >= 'a' && c <= 'g') {
            // lower case
            index = c - 97;
        } else if (c >= 'A' && c <= 'g') {
            // upper case
            index = c - 65;
        }
    }
    if (index == -1) {
        throw std::invalid_argument("Invalid note: " + str);
    }
    const static int indexMap[7] = {0 /*A*/, 2 /*B*/, 3 /*C*/, 5 /*D*/, 7 /*E*/, 8 /*F*/, 10 /*G*/};

    absoluteNote = indexMap[index];

    //has accidental
    if (str.size() >= 2) {
        switch (str[0]) {
            //accidental is before note (LDP-style) (-,--,+,x)
            case '-':
                if (str.size() >= 3 && str[1] == '-') {
                    shiftSemitone(-2, DOUBLE_FLAT);
                } else {
                    shiftSemitone(-1, FLAT);
                }
                break;
            case '+':
                shiftSemitone(1, SHARP);
                break;
            case 'x':
                shiftSemitone(2, DOUBLE_SHARP);
                break;

            //accidental is after note (b,bb,#,x)
            default:
                switch (str[1]) {
                    case 'b':
                        if (str.size() >= 3 && str[2] == 'b') {
                            shiftSemitone(-2, DOUBLE_FLAT);
                        } else {
                            shiftSemitone(-1, FLAT);
                        }
                        break;
                    case '#':
                        shiftSemitone(1, SHARP);
                        break;
                    case 'x':
                        shiftSemitone(2, DOUBLE_SHARP);
                        break;
                    default:
                        throw std::invalid_argument("Invalid note: " + str);
                }
                break;
        }
    } else {
        updateString();
    }
}

ChordNamer::Note &ChordNamer::Note::shiftSemitone(const int32_t semitones, const Accidental defaultAccidental) {
    const uint32_t absShift = (semitones > 0) ? semitones : 12 + semitones;
    absoluteNote = static_cast<int32_t>(absoluteNote + absShift) % 12;

    switch (absoluteNote) {
        case 1: // A#/Bb
        case 4: // C#/Db
        case 6: // D#/Eb
        case 9: // F#/Gb
        case 11: // G#/Ab
            if (defaultAccidental == NATURAL)
                accidental = preferredAccidental;
            else
                accidental = defaultAccidental;
            break;
        default:
            accidental = defaultAccidental;
    }

    updateString();
    return *this;
}

ChordNamer::Note &ChordNamer::Note::respell() {
    // reduce from double sharp/flat
    if (accidental == DOUBLE_FLAT || accidental == DOUBLE_SHARP) {
        accidental = NATURAL; // if resulting note requires (#/b) accidental, it will be handled below
    }
    switch (absoluteNote) {
        //No accidental notes
        case 0: //A
        case 5: //D
        case 10: //G
            break; //do nothing

        //Special accidental notes
        case 3: //C <-> B#
        case 8: //F <-> E#
            accidental = (accidental == NATURAL) ? SHARP : NATURAL;
            break;
        case 7: //E <-> Fb
        case 2: //B <-> Cb
            accidental = (accidental == NATURAL) ? FLAT : NATURAL;
            break;

        default: //switch between sharp and flat, or the preferred accidental
            switch (accidental) {
                case SHARP:
                    accidental = FLAT;
                    break;
                case FLAT:
                    accidental = SHARP;
                    break;
                case NATURAL:
                    accidental = preferredAccidental;
                    break;
                default:
                    break;
            }
        //accidental = (accidental == SHARP) ? FLAT : SHARP;
            break;
    }

    updateString();
    return *this;
}

std::string ChordNamer::Note::toString() const {
    return strNote;
}

const char *ChordNamer::Note::toCString() const {
    return strNote.c_str();
}

bool ChordNamer::Note::validate(const std::string &strNote) {
    if (strNote.empty()) {
        return false;
    }

    //If letter first
    if (char c = strNote[0]; (c >= 'a' && c <= 'g') || (c >= 'A' && c <= 'G')) {
        //If flat
        if (strNote.size() > 1 && strNote[1] == 'b') {
            if (strNote.size() > 2 && strNote[2] == 'b') {
                return true;
            }
            return strNote.size() == 2;
        }

        //If sharp or double sharp
        if (strNote.size() > 1 && (strNote[1] == '#' || strNote[1] == 'x')) {
            return strNote.size() == 2;
        }

        return strNote.size() == 1;
    }

    //If sharp or double sharp first
    if (char c = strNote[0]; c == '+' || c == 'x') {
        return strNote.size() == 2 && ((strNote[1] >= 'a' && strNote[1] <= 'g') || (
                                           strNote[1] >= 'A' && strNote[1] <= 'G'));
    }

    // If flat first
    if (strNote[0] == '-') {
        if (strNote.size() > 1 && strNote[1] == '-') {
            return strNote.size() == 3 && ((strNote[2] >= 'a' && strNote[2] <= 'g') || (
                                               strNote[2] >= 'A' && strNote[2] <= 'G'));
        }
        return strNote.size() == 2 && ((strNote[1] >= 'a' && strNote[1] <= 'g') || (
                                           strNote[1] >= 'A' && strNote[1] <= 'G'));
    }

    return false;
}

uint32_t ChordNamer::Note::getDistanceTo(const Note &right) const {
    return (right.absoluteNote - this->absoluteNote + 12) % 12;
}

ChordNamer::Note ChordNamer::Note::getNoteFromDistance(int semitone) const {
    Note target(*this);
    return target.shiftSemitone(semitone);
}

std::vector<ChordNamer::Note> ChordNamer::Note::extractUnique(const std::vector<Note> &allNotes) {
    std::vector<Note> unique;
    bool bNotes[12] = {false};

    for (const Note &currNote: allNotes) {
        if (!bNotes[currNote.absoluteNote]) {
            bNotes[currNote.absoluteNote] = true;
            unique.push_back(currNote);
        }
    }
    return unique;
}

std::vector<uint32_t> ChordNamer::Note::getUniqueIndexes(const std::vector<Note> &allNotes) {
    std::vector<uint32_t> uniqueIndex;
    bool bNotes[12] = {false};

    for (size_t i = 0; i < allNotes.size(); i++) {
        if (!bNotes[allNotes[i].absoluteNote]) {
            bNotes[allNotes[i].absoluteNote] = true;
            uniqueIndex.push_back(i);
        }
    }

    return uniqueIndex;
}

void ChordNamer::Note::updateString() {
    //the magic happens here

    switch ((absoluteNote - accidental + 12) % 12) {
        case 0:
            strNote = "A";
            break;
        case 2:
            strNote = "B";
            break;
        case 3:
            strNote = "C";
            break;
        case 5:
            strNote = "D";
            break;
        case 7:
            strNote = "E";
            break;
        case 8:
            strNote = "F";
            break;
        case 10:
            strNote = "G";
            break;
        default:
            break;
    }

    switch (accidental) {
        case SHARP:
            strNote += "#";
            break;
        case FLAT:
            strNote += "b";
            break;
        case DOUBLE_SHARP:
            strNote += "x";
            break;
        case DOUBLE_FLAT:
            strNote += "bb";
            break;
        case NATURAL:
        default:
            break;
    }
}
