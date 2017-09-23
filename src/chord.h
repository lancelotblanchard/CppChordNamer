#ifndef CHORD_H
#define CHORD_H
#include <string>
#include <vector>

#include "interval.h"
#include "note.h"

using std::string;
using std::vector;

class chord : public interval {
	vector<string> chordNames;

public:

	chord() {}

	chord(const vector<string> &allNotes): interval(allNotes) {
		evaluateAllPossibleChordNames();
	}

	chord(const vector<note> &allNotes): interval(allNotes) {
		evaluateAllPossibleChordNames();
	}

	chord(const string &line) : interval(line) {
		evaluateAllPossibleChordNames();
	}

	void reset(const vector<string> &allNotes) override {
		interval::reset(allNotes);
		chordNames.clear();
		evaluateAllPossibleChordNames();
	}

	void reset(const string &line) override {
		interval::reset(line);
		chordNames.clear();
		evaluateAllPossibleChordNames();
	}

	void reset(const vector<note> &allNotes) override {
		interval::reset(allNotes);
		chordNames.clear();
		evaluateAllPossibleChordNames();
	}

	static string getChordQuality(const vector<note> &allNotes, Dint current_root, int *ranking = nullptr) {
		size_t total_count = allNotes.size();

		note root = allNotes[current_root];

		vector<Dint> distances; //in terms of semitones

		for (size_t k = 0; k < total_count; k++) {
			note current_note = allNotes[(current_root + k) % total_count];
			distances.push_back(root.getDistanceTo(current_note));
		}

		return getChordQuality(distances, ranking);
	}

	static string getChordQuality(const vector<Dint> &distances, int *ranking = nullptr) {

		Dint extStack[4] = { M7, M2, P4, M6 }; //default 7 9 11 13

		vector<string> intervalList = getIntervalList(distances, true);

		vector<string> additional;

		string quality;
		string sus;

		bool isdim = false;

		bool exist[12] = { false };
		for (Dint dist : distances) {
			exist[dist] = true;
		}

		if (check(exist[M3])) { //major chord
			if (!isAltered(exist) && check(exist[A5])) { //augmented chord
				quality += "aug";
			}
			exist[P5] = false;

		} else if (check(exist[m3])) { //minor chord

			if (!check(exist[P5]) && check(exist[D5])) { //dim chord
				isdim = true;

				if (exist[m7]) { //mXb5 (halfdim)
					quality += "m";
					additional.push_back("b5");

				} else {
					quality += "dim"; //dim
				}

			} else {
				quality += "m"; //normal minor chord
			}

		} else { //suspended chord
			exist[P5] = false;
			if (check(exist[P4])) {
				sus += "sus4";
			} else if (check(exist[M2])) {
				sus += "sus2";
			} else {
				additional.push_back("omit3");
			}
		}

		string extension = getExtensions(exist, extStack, intervalList, isdim);
		if (extension == "") {
			//check 6 chords
			if (check(exist[M6])) {
				quality += "6";
				if (check(exist[M9])) { //check 6/9 chord
					quality += "/9";
				}
			}
			
		}

		quality += extension;
		quality += sus;

		//dump all the rest of the notes not checked (leftover) to additional
		for (int i = 1; i < 12; i++) {
			if (check(exist[i])) {
				additional.push_back(intervalList[i]);
			}
		}

		if (additional.size() == 1) {
			if (additional[0].substr(0, 4) == "omit")
				quality += "(" + additional[0] + ")";
			else if (additional[0][0] == '#' || additional[0][0] == 'b')
				quality += additional[0];
			else
				quality += "add" + additional[0];

		} else if (additional.size() > 1) {
			quality += "(" + additional[0];
			for (size_t i = 1; i < additional.size(); i++) {
				quality += ", " + additional[i];
			}
			quality += ")";
		}

		if (ranking != nullptr) { //the lower the number, the simple the chord name is
			*ranking = additional.size() + (sus.size() > 0); //sus has weight 1
		}
		return quality;
	}

	void printChord(Dint note_index) const {
		printf("Chord: %s\n\n\n", chordNames[note_index].c_str());
	}

	void printAllPossible() const override {
		int count = uniqueIndexes.size();
		for (int i = 0; i < count; i++) {
			printInterval(uniqueIndexes[i]);
			printChord(i);
		}
	}

private:

	void evaluateAllPossibleChordNames() {
		vector<int> ranking;

		for (Dint i : uniqueIndexes) {
			ranking.push_back(evaluateChordName(i));
		}

		/*
		Sort the chord names based on their ranking (complexity of the name),
		so that the least complex chord name is shown first and most complex name 
		is shown last.
		*/
		insertion_sort_chordNames(ranking);
	}

	int evaluateChordName(Dint current_root) {
		int total_count = allNotes.size();

		//currently is just the number of additional intervals (+ 1 if is sus chord)
		//The lower the better
		int ranking; 

		string chordName = allNotes[current_root].toString() + getChordQuality(allNotes, current_root, &ranking);

		if (current_root != 0) { //not in root position (slash chord)
			chordName += "/" + allNotes[0].toString();
		}

		this->chordNames.push_back(chordName);

		return ranking;
	}

	/*
	chordNames, uniqueIndexes, and ranking all corresponds with each other,
	so they have to be swapped together
	*/
	void swap(int x, int y, vector<int> &ranking) {
		string tmp_str = chordNames[x];
		chordNames[x] = chordNames[y];
		chordNames[y] = tmp_str;

		Dint tmp_dint = uniqueIndexes[x];
		uniqueIndexes[x] = uniqueIndexes[y];
		uniqueIndexes[y] = tmp_dint;

		int tmp_int = ranking[x];
		ranking[x] = ranking[y];
		ranking[y] = tmp_int;
	}

	/*
	Sort all the chords based on their ranking (complexity) using insertion sort
	*/
	void insertion_sort_chordNames(vector<int> &ranking) {
		int count = ranking.size();
		for (int i = 1; i < count; i++) {
			for (int j = i; j > 0; j--) {
				if (ranking[j] < ranking[j - 1]) {
					swap(j, j - 1, ranking);
				} else {
					break;
				}
			}
		}
	}

	static string getExtensions(bool *exist, Dint extStack[], const vector<string> &intervalList, bool isdim = false) {
		string ext;
		if (check(exist[M7])) { //maj7
			ext = "maj";
		} else if (check(exist[m7])) { //dom7
			//nothing
		} else if (isdim && check(exist[M6])) { //dim7
			extStack[0] = M6; //dim7 == maj6
			extStack[3] = m6; //change 13 to b13
		} else {
			return "";
		}

		Dint highest = extStack[0];
		for (int i = 1; i < 4; i++) {
			Dint curr_ext = extStack[i];
			if (check(exist[curr_ext])) {
				highest = curr_ext;
			}
		}

		return ext + intervalList[highest];
	}

	static bool isAltered(bool *exist) {
		if (exist[M3] && exist[A5] && !(exist[m2] || exist[m3] || exist[D5])) {
			return false; //only #5 is present, so is aug chord
		}
		return (exist[m2] || exist[m3] || exist[D5] || exist[A5]);
	}

	//return the value of val and mark val as false
	static inline bool check(bool &val) {
		bool ret = val;
		val = false;
		return ret;
	}
};

#endif