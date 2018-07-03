#ifndef NOTE_H
#define NOTE_H

#include <string>
#include <vector>

using std::string;
using std::vector;

/*
0  A
1  A#/Bb
2  B/Cb  *
3  C/B#  *
4  C#/Db
5  D
6  D#/Eb
7  E/Fb  *
8  F/E#  *
9  F#/Gb
10 G
11 G#/Ab
*/

class note {
public:
	enum Accidental
	{
		DOUBLE_FLAT = -2, FLAT = -1, NATURAL = 0, SHARP = 1, DOUBLE_SHARP = 2
	};

private:
    int absolute_note; // A == 0, A# == 1, ... , G# == 11

    Accidental accidental;
    Accidental preferred_accidental; //the default accidental to use when not provided any

	string str_note;

public:
	note(string str, Accidental preferred_accidental = SHARP)
        : accidental(NATURAL), preferred_accidental(preferred_accidental) 
    {
		if (!validate(str)) {
			throw "Invalid notes!";
		}

		int index;
		if (str[0] >= 97) { // lower case
			index = str[0] - 97;
		} else { // upper case
			index = str[0] - 65;
		}
		const static int indexMap[7] = { 0 /*A*/,2 /*B*/,3 /*C*/,5 /*D*/,7 /*E*/,8 /*F*/,10 /*G*/ };

		absolute_note = indexMap[index];

		if (str.size() >= 2) { //has accidental
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
			}
        } else {
            updateString();
        }
	}

	note& shift_semitone(int semitones, Accidental default_accidental = NATURAL) {
		unsigned int abs_shift = (semitones > 0) ? semitones : 12 + semitones;
		absolute_note = (absolute_note + abs_shift) % 12;
		
		switch (absolute_note) {
		case 1:  // A#/Bb
		case 4:  // C#/Db
		case 6:  // D#/Eb
		case 9:  // F#/Gb
		case 11: // G#/Ab
            if (default_accidental == NATURAL)
                accidental = preferred_accidental;
            else
                accidental = default_accidental;
			break;
		default:
            accidental = default_accidental;
		}

		updateString();
		return *this;
	}

	note& respell() {

        // reduce from double sharp/flat
        if (accidental == DOUBLE_FLAT || accidental == DOUBLE_SHARP) {
            accidental = NATURAL; // if resulting note requires (#/b) accidental, it will be handled below
        }
		switch (absolute_note)
		{
			//No accidental notes
		case 0:  //A
		case 5:  //D
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
                accidental = preferred_accidental;
                break;
            }
			//accidental = (accidental == SHARP) ? FLAT : SHARP;
			break;
		}

		updateString();
		return *this;
	}

	string toString() const {
		return str_note;
	}

	const char *toCString() const {
		return str_note.c_str();
	}

	static bool validate(string str_note) {
		if (str_note.size() == 0) {
			return false;
		}

		char firstChar = str_note[0];
		if (firstChar < 'A') {
			return false;
		} else if (firstChar > 'G' && firstChar < 'a') { //between 'G' and 'a' exclusive
			return false;
		} else if (firstChar > 'g') {
			return false;
		}

		if (str_note.size() > 1) {
			if (str_note[1] != 'b' && str_note[1] != '#' && str_note[1] != 'x') {
				return false;
			}
		}

		return true;
	}

	//distance between two notes in terms of semitone count
	unsigned int getDistanceTo(const note &right) const {
		return (right.absolute_note - this->absolute_note + 12) % 12;
	}

	note getNoteFromDistance(int semitone) const {
		note target(*this);
		return target.shift_semitone(semitone);
	}

	static vector<note> extractUnique(const vector<note> &allNotes) {
		vector<note> unique;
		bool bNotes[12] = { false };

		for (note curr_note : allNotes) {
			if (!bNotes[curr_note.absolute_note]) {
				bNotes[curr_note.absolute_note] = true;
				unique.push_back(curr_note);
			}
		}
		return unique;
	}

	static vector<unsigned int> getUniqueIndexes(const vector<note> &allNotes) {
		vector<unsigned int> uniqueIndex;
		bool bNotes[12] = { false };

		for (size_t i = 0; i < allNotes.size(); i++) {
			if (!bNotes[allNotes[i].absolute_note]) {
				bNotes[allNotes[i].absolute_note] = true;
				uniqueIndex.push_back(i);
			}
		}

		return uniqueIndex;
	}

private:
	void updateString() {
		//the magic happens here
		int curr_abs_note = (absolute_note - accidental + 12) % 12;

		switch (curr_abs_note) {
		case 0:
			str_note = "A";
			break;
		case 2:
			str_note = "B";
			break;
		case 3:
			str_note = "C";
			break;
		case 5:
			str_note = "D";
			break;
		case 7:
			str_note = "E";
			break;
		case 8:
			str_note = "F";
			break;
		case 10:
			str_note = "G";
			break;
		}

		switch (accidental) {
		case SHARP:
			str_note += "#";
			break;
		case FLAT:
			str_note += "b";
			break;
        case DOUBLE_SHARP:
            str_note += "x";
            break;
        case DOUBLE_FLAT:
            str_note += "bb";
            break;
		}
	}
};

#endif