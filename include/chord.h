#pragma once

#include <string>
#include <vector>

#include "interval.h"
#include "note.h"

namespace ChordNamer {
	class Chord : public Interval {
	public:
		Chord() = default;

		explicit Chord(const std::vector<std::string> &allNotes);

		explicit Chord(const std::vector<Note> &allNotes);

		explicit Chord(const std::string &line);

		Chord &reset(const std::vector<std::string> &allNotes) override;

		Chord &reset(const std::string &line) override;

		Chord &reset(const std::vector<Note> &allNotes) override;

		static std::string getChordQualityFromNotes(const std::vector<Note> &allNotes, uint32_t current_root,
		                                            int32_t *ranking = nullptr);

		static std::string getChordQualityFromDists(const std::vector<uint32_t> &distances, int32_t *ranking = nullptr);

		std::vector<std::string> chordNames;

	private:
		void evaluateAllPossibleChordNames();

		int32_t evaluateChordName(uint32_t current_root);

		/*
		chordNames, uniqueIndexes, and ranking all corresponds with each other,
		so they have to be swapped together
		*/
		void swap(int32_t x, int32_t y, std::vector<int32_t> &ranking);

		/*
		Sort all the chords based on their ranking (complexity) using insertion sort
		*/
		void insertionSortChordNames(std::vector<int32_t> &ranking);

		static std::string getExtensions(bool *exist, uint32_t extStack[], const std::vector<std::string> &intervalList,
		                                 bool isdim = false);

		static bool isAltered(const bool *exist);

		/* return the value of val and mark val as false */
		static inline bool check(bool &val);
	};
};
