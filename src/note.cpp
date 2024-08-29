#include "note.h"

ChordNamer::Note::Note(const std::string &str, const Accidental preferredAccidental): accidental(NATURAL),
    preferredAccidental(preferredAccidental) {
    if (!validate(str)) {
        throw std::invalid_argument("Invalid notes!");
    }

    int index;
    if (str[0] >= 97) {
        // lower case
        index = str[0] - 97;
    } else {
        // upper case
        index = str[0] - 65;
    }
    const static int indexMap[7] = {0 /*A*/, 2 /*B*/, 3 /*C*/, 5 /*D*/, 7 /*E*/, 8 /*F*/, 10 /*G*/};

    absoluteNote = indexMap[index];

    if (str.size() >= 2) {
        //has accidental
        switch (str[1]) {
            case 'b':
                if (str.size() >= 3 && str[2] == 'b') {
                    shift_semitone(-2, DOUBLE_FLAT);
                } else {
                    shift_semitone(-1, FLAT);
                }
                break;
            case '#':
                shift_semitone(1, SHARP);
                break;
            case 'x':
                shift_semitone(2, DOUBLE_SHARP);
                break;
            default:
                break;
        }
    } else {
        updateString();
    }
}

ChordNamer::Note &ChordNamer::Note::shift_semitone(const int32_t semitones, const Accidental default_accidental) {
    const uint32_t abs_shift = (semitones > 0) ? semitones : 12 + semitones;
    absoluteNote = static_cast<int32_t>(absoluteNote + abs_shift) % 12;

    switch (absoluteNote) {
        case 1: // A#/Bb
        case 4: // C#/Db
        case 6: // D#/Eb
        case 9: // F#/Gb
        case 11: // G#/Ab
            if (default_accidental == NATURAL)
                accidental = preferredAccidental;
            else
                accidental = default_accidental;
            break;
        default:
            accidental = default_accidental;
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

    if (char firstChar = strNote[0]; firstChar < 'A' || (firstChar > 'G' && firstChar < 'a') || firstChar > 'g') {
        return false;
    }

    if (strNote.size() > 1) {
        if (strNote[1] != 'b' && strNote[1] != '#' && strNote[1] != 'x') {
            return false;
        }
    }

    return true;
}

uint32_t ChordNamer::Note::getDistanceTo(const Note &right) const {
    return (right.absoluteNote - this->absoluteNote + 12) % 12;
}

ChordNamer::Note ChordNamer::Note::getNoteFromDistance(int semitone) const {
    Note target(*this);
    return target.shift_semitone(semitone);
}

std::vector<ChordNamer::Note> ChordNamer::Note::extractUnique(const std::vector<Note> &allNotes) {
    std::vector<Note> unique;
    bool bNotes[12] = {false};

    for (const Note &curr_note: allNotes) {
        if (!bNotes[curr_note.absoluteNote]) {
            bNotes[curr_note.absoluteNote] = true;
            unique.push_back(curr_note);
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
