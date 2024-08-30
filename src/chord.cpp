#include <chord.h>

ChordNamer::Chord::Chord(const std::vector<std::string> &allNotes) : Interval(allNotes) {
	evaluateAllPossibleChordNames();
}

ChordNamer::Chord::Chord(const std::vector<Note> &allNotes): Interval(allNotes) {
	evaluateAllPossibleChordNames();
}

ChordNamer::Chord::Chord(const std::string &line) : Interval(line) {
	evaluateAllPossibleChordNames();
}

ChordNamer::Chord &ChordNamer::Chord::reset(const std::vector<std::string> &allNotes) {
	Interval::reset(allNotes);
	chordNames.clear();
	evaluateAllPossibleChordNames();
	return *this;
}

ChordNamer::Chord &ChordNamer::Chord::reset(const std::string &line) {
	Interval::reset(line);
	chordNames.clear();
	evaluateAllPossibleChordNames();
	return *this;
}

ChordNamer::Chord &ChordNamer::Chord::reset(const std::vector<Note> &allNotes) {
	Interval::reset(allNotes);
	chordNames.clear();
	evaluateAllPossibleChordNames();
	return *this;
}

std::string ChordNamer::Chord::getChordQualityFromNotes(const std::vector<Note> &allNotes, const uint32_t currentRoot,
                                                        int32_t *ranking) {
	const size_t totalCount = allNotes.size();

	const Note &root = allNotes[currentRoot];

	std::vector<uint32_t> distances; //in terms of semitones

	for (size_t k = 0; k < totalCount; k++) {
		const Note &currentNote = allNotes[(currentRoot + k) % totalCount];
		distances.push_back(root.getDistanceTo(currentNote));
	}

	return getChordQualityFromDists(distances, ranking);
}

std::string ChordNamer::Chord::getChordQualityFromDists(const std::vector<uint32_t> &distances, int32_t *ranking) {
	uint32_t extStack[4] = {M7, M9, P11, M13}; //default 7 9 11 13

	std::vector<std::string> intervalList = getIntervalList(distances, true);

	std::vector<std::string> additional;

	std::string quality;
	std::string sus;

	bool isdim = false;

	bool exist[12] = {false};
	for (uint32_t dist: distances) {
		exist[dist] = true;
	}

	if (check(exist[M3])) {
		//major chord
		if (!isAltered(exist) && check(exist[A5])) {
			//augmented chord
			quality += "aug";
		}
		exist[P5] = false;
	} else if (check(exist[m3])) {
		//minor chord

		if (!check(exist[P5]) && check(exist[d5])) {
			//dim chord
			isdim = true;

			if (exist[m7]) {
				//mXb5 (halfdim)
				quality += "m";
				additional.emplace_back("b5");
			} else {
				quality += "dim"; //dim
			}
		} else {
			quality += "m"; //normal minor chord
		}
	} else {
		//suspended chord
		if (check(exist[P4])) {
			sus += "sus4";
		} else if (check(exist[M2])) {
			sus += "sus2";
		} else if (check(exist[P5])) {
			quality += "5";
		} else {
			additional.emplace_back("omit3");
		}
		exist[P5] = false;
	}

	std::string extension = getExtensions(exist, extStack, intervalList, isdim);
	if (extension.empty()) {
		//check 6 chords
		if (check(exist[M6])) {
			quality += "6";
			if (check(exist[M9])) {
				//check 6/9 chord
				quality += "/9";
			}
		}
	}

	quality += extension;
	quality += sus;

	//dump all the rest of the notes not checked (leftover) to additional
	for (int32_t i = 1; i < 12; i++) {
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

	if (ranking != nullptr) {
		//the lower the number, the simple the chord name is
		*ranking = static_cast<int32_t>(additional.size()) + sus.empty(); //sus has weight 1
	}
	return quality;
}

void ChordNamer::Chord::evaluateAllPossibleChordNames() {
	std::vector<int32_t> ranking;
	ranking.reserve(uniqueIndexes.size());

	for (const uint32_t i: uniqueIndexes) {
		ranking.push_back(evaluateChordName(i));
	}

	/*
	Sort the chord names based on their ranking (complexity of the name),
	so that the least complex chord name is shown first and most complex name
	is shown last.
	*/
	insertionSortChordNames(ranking);
}

int32_t ChordNamer::Chord::evaluateChordName(uint32_t currentRoot) {
	//currently is just the number of additional intervals (+ 1 if is sus chord)
	//The lower the better
	int32_t ranking;

	std::string chordName = allNotes[currentRoot].toString();
	std::string chordQuality = getChordQualityFromNotes(allNotes, currentRoot, &ranking);

	if (currentRoot != 0) {
		//not in root position (slash chord)

		if (allNotes.size() > 1) {
			int32_t rootlessRanking;

			std::vector<Note> rootless_allNotes = allNotes;
			rootless_allNotes[0] = rootless_allNotes[1]; // "hide" the root

			std::string rootlessChordQuality = getChordQualityFromNotes(
				rootless_allNotes, currentRoot, &rootlessRanking);

			// check if rootless chord quality is shorter (simpler)
			// TODO: can include as additional optional chord name instead of replacing it
			if (rootlessChordQuality.size() < chordQuality.size()) {
				chordQuality = rootlessChordQuality;
				ranking = rootlessRanking;
			}
		}

		chordQuality += "/" + allNotes[0].toString();
		ranking++;
	}

	chordName += chordQuality;

	this->chordNames.push_back(chordName);

	return ranking;
}

void ChordNamer::Chord::swap(int32_t x, int32_t y, std::vector<int32_t> &ranking) {
	std::string tmpStr = chordNames[x];
	chordNames[x] = chordNames[y];
	chordNames[y] = tmpStr;

	uint32_t tmpDint = uniqueIndexes[x];
	uniqueIndexes[x] = uniqueIndexes[y];
	uniqueIndexes[y] = tmpDint;

	int32_t tmpInt = ranking[x];
	ranking[x] = ranking[y];
	ranking[y] = tmpInt;
}

void ChordNamer::Chord::insertionSortChordNames(std::vector<int32_t> &ranking) {
	const auto count = static_cast<int32_t>(ranking.size());
	for (int32_t i = 1; i < count; i++) {
		for (int32_t j = i; j > 0; j--) {
			// chord name sizes act as tie breaker
			if ((ranking[j] == ranking[j - 1] && chordNames[j].size() < chordNames[j - 1].size())
			    || (ranking[j] < ranking[j - 1])) {
				swap(j, j - 1, ranking);
			} else {
				break;
			}
		}
	}
}

std::string ChordNamer::Chord::getExtensions(bool *exist, uint32_t extStack[],
                                             const std::vector<std::string> &intervalList,
                                             const bool isdim) {
	std::string ext;
	if (check(exist[M7])) {
		//maj7
		ext = "maj";
	} else if (check(exist[m7])) {
		//dom7
		//nothing
	} else if (isdim && check(exist[d7])) {
		//dim7
		extStack[0] = d7; //dim7 == maj6
		extStack[3] = m6; //change 13 to b13
	} else {
		return "";
	}

	uint32_t highest = extStack[0];
	for (int32_t i = 1; i < 4; i++) {
		uint32_t currExt = extStack[i];
		if (check(exist[currExt])) {
			highest = currExt;
		}
	}

	return ext + intervalList[highest];
}

bool ChordNamer::Chord::isAltered(const bool *exist) {
	if (exist[M3] && exist[A5] && !(exist[m2] || exist[A2] || exist[d5])) {
		return false; //only #5 is present, so is aug chord
	}
	return (exist[m2] || exist[A2] || exist[d5] || exist[A5]);
}

bool ChordNamer::Chord::check(bool &val) {
	bool ret = val;
	val = false;
	return ret;
}
