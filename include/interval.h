#pragma once

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

/*
INTERVAL MACRO CONVENTIONS:
    m : minor
    M : major
    P : perfect
    d : diminished
    A : augmented
*/

// basic
#define m2 1	//minor 2nd
#define M2 2	//major 2nd
#define m3 3	//minor 3rd
#define M3 4	//major 3rd
#define d4 4    //diminished 4th
#define P4 5	//perfect 4th
#define A4 6	//augmented 4th
#define d5 6	//diminished 5th
#define P5 7	//perfect 5th
#define A5 8	//augmented 5th
#define m6 8	//minor 6th
#define M6 9	//major 6th
#define m7 10	//minor 7th
#define M7 11	//major 7th

// extensions
#define m9 m2	//minor 9th
#define M9 M2	//major 9th
#define P11 P4  //perfect 11th
#define A11 A4  //augmented 11th (#11)
#define m13 m6  //minor 13th (b13)
#define M13 M6  //major 13th

// extras
#define A2 m3   //augmented 2nd (#2)
#define d6 P5   //diminished 6th (bb6)
#define A6 m7   //augmented 6th (#6)
#define d7 M6   //diminished 7th (bb7)

#include <string>
#include <vector>
#include <cstdint>

#include "note.h"

namespace ChordNamer {
	class Interval {
	protected:
		std::vector<Note> allNotes;
		std::vector<uint32_t> uniqueIndexes; //index in notes

	public:
		Interval() = default;

		virtual ~Interval() = default;

		explicit Interval(const std::string &line);

		explicit Interval(const std::vector<std::string> &allNotes);

		explicit Interval(const std::vector<Note> &allNotes);

		virtual Interval &reset(const std::string &line);

		virtual Interval &reset(const std::vector<std::string> &allNotes);

		virtual Interval &reset(const std::vector<Note> &allNotes);

		/*
		Process the all the distances from the current root and output the interval list
		*/
		static std::vector<std::string> getIntervalList(const std::vector<uint32_t> &distances, bool chordMode = false);

	private:
		static std::vector<std::string> distancesToIntervals(const std::vector<uint32_t> &distances);

		void split(const std::string &line);
	};
}
