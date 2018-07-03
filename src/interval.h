#ifndef INTERVAL_HANDLER_H
#define INTERVAL_HANDLER_H

/*
 index | interval
-------+----------
   0   |   1
   1   |   b9
   2   |   9
   3   |   b3
   4   |   3
   5   |   11
   6   |   b5
   7   |   5
   8   |   #5
   9   |   6
  10   |   b7
  11   |   7
*/

#define m2 1	//minor 2nd
#define M2 2	//major 2nd
#define m3 3	//minor 3rd
#define M3 4	//major 3rd
#define P4 5	//perfect 4th
#define A4 6	//augmented 4th
#define D5 6	//diminished 5th
#define P5 7	//perfect 5th
#define A5 8	//augmented 5th
#define m6 8	//minor 6th
#define M6 9	//major 6th
#define m7 10	//minor 7th
#define M7 11	//major 7th
#define m9 1	//minor 9th
#define M9 2	//major 9th

#include <string>
#include <vector>

#include "note.h"

using std::string;
using std::vector;

typedef unsigned int Dint; //distance type

class interval {

protected:
	vector<note> allNotes;
	vector<Dint> uniqueIndexes; //index in notes

public:
	interval() {}

	interval(const string &line) {
		reset(line);
	}

	interval(const vector<string> &allNotes) {
		reset(allNotes);
	}

	interval(const vector<note> &allNotes): allNotes(allNotes) {
		uniqueIndexes = note::getUniqueIndexes(allNotes);
	}

	virtual interval& reset(const string &line) {
		split(line);
		uniqueIndexes = note::getUniqueIndexes(allNotes);
        return *this;
	}

	virtual interval& reset(const vector<string> &allNotes) {
		this->allNotes.clear();
		//convert strings to notes
		for (string note_str : allNotes) {
			this->allNotes.push_back(note_str);
		}
		uniqueIndexes = note::getUniqueIndexes(this->allNotes);
        return *this;
	}

	virtual interval& reset(const vector<note> &allNotes) {
		this->allNotes = allNotes;
		uniqueIndexes = note::getUniqueIndexes(allNotes);
        return *this;
	}

	void printInterval(Dint current_root) const {
		int total_count = allNotes.size(); //total number of input notes
		note root = allNotes[current_root];
		vector<Dint> distances; //in terms of semitones

		//print notes and store semitone distances
		for (int k = 0; k < total_count; k++) {
			note current_note = allNotes[(current_root + k) % total_count];
			printf("%s\t", current_note.toCString());
			distances.push_back(root.getDistanceTo(current_note));
		}
		puts("\n");

		//print intervals
		vector<string> intervalList = distances_to_intervals(distances);
		for (string interval : intervalList) {
			printf("%s\t", interval.c_str());
		}
		puts("\n");
	}

	virtual void printAllPossible() const {
		for (Dint i : uniqueIndexes) {
			printInterval(i);
		}
	}
	
	/*
	Process the all the distances from the current root and output the interval list
	*/
	static vector<string> getIntervalList(const vector<Dint> &distances, bool chordMode = false) {
		vector<string> possible_intervals = { "1", "b9", "9", "b3", "3", "11", "b5", "5", "#5", "6", "b7", "7" };

		bool exist[12] = { false };
		//below compare with semitone interval
		//if 3 and b3 is present, b3 becomes #9
		//if 5 and (b5 || #5) is present, (#11 || b13) is used
		//if 7 is present, 6 becomes 13
		//if b3/3 is NOT present, 4 is used instead of 11
		//if both b3/3 and 4 are NOT present, then 2 is used instead of 9
		//if b3 and b5 and 6 are present (diminished) , then 6/13 becomes 7 and b6/b13 becomes 13

		bool thirdPresent = false;

		for (Dint distance : distances) {
			exist[distance] = true;
		}

		if (exist[M3]) { //major 3rd
			possible_intervals[m3] = "#9";
			thirdPresent = true;
		} 
		if (exist[P5]) { //perfect 5th
			possible_intervals[D5] = "#11";
			possible_intervals[m6] = "b13";
		} 
		if (exist[m7] || exist[M7]) { //minor or major 7th
			possible_intervals[M6] = "13";
		} 
		if (exist[m3]) { //minor 3rd
			thirdPresent = true;
			if (exist[D5] && exist[M6] && !exist[m7] && !exist[M7]) { //full diminished chord
				if (chordMode) {
					possible_intervals[M6] = "7";
					possible_intervals[m6] = "13";
				} else {
					possible_intervals[M6] = "bb7";
					possible_intervals[m6] = "b13";
				}
			}
		}

		if (!thirdPresent) { //sus chord
			possible_intervals[P4] = "4";

			if (!exist[P4])
				possible_intervals[M2] = "2";
		}

		return possible_intervals;
	}

private:

	static vector<string> distances_to_intervals(const vector<Dint> &distances) {
		vector<string> intervalList = getIntervalList(distances);

		vector<string> intervals; //actual intervals present in the input list

		for (Dint semitone : distances) {
			intervals.push_back(intervalList[semitone]);
		}
		return intervals;
	}

	void split(const string &line) {
		this->allNotes.clear();
		string note_str;
		for (char c : line) {
			if ((c == ' ' || c == ',')) { //encounters delimiter
				if (note_str.size() > 0) {
					allNotes.push_back(note_str);
					note_str.clear();
				}
			} else {
				note_str += c;
			}
		}
		if (note_str.size() > 0) {
			allNotes.push_back(note_str);
		}
	}
	
};

#endif