#pragma once

#include <string>
#include <vector>

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

namespace ChordNamer {
	class Note {
	public:
		enum Accidental {
			DOUBLE_FLAT = -2, FLAT = -1, NATURAL = 0, SHARP = 1, DOUBLE_SHARP = 2
		};

		explicit Note(const std::string &str, Accidental preferredAccidental = SHARP);

		Note &shift_semitone(int32_t semitones, Accidental default_accidental = NATURAL);

		Note &respell();

		[[nodiscard]] std::string toString() const;

		[[nodiscard]] const char *toCString() const;

		static bool validate(const std::string &strNote);

		//distance between two notes in terms of semitone count
		[[nodiscard]] uint32_t getDistanceTo(const Note &right) const;

		[[nodiscard]] Note getNoteFromDistance(int semitone) const;

		static std::vector<Note> extractUnique(const std::vector<Note> &allNotes);

		static std::vector<uint32_t> getUniqueIndexes(const std::vector<Note> &allNotes);

	private:
		void updateString();

		int32_t absoluteNote; // A == 0, A# == 1, ... , G# == 11

		Accidental accidental;
		Accidental preferredAccidental; //the default accidental to use when not provided any

		std::string strNote;
	};
}
