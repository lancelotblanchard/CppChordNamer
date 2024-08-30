#include <interval.h>

ChordNamer::Interval::Interval(const std::string &line) {
    reset(line);
}

ChordNamer::Interval::Interval(const std::vector<std::string> &allNotes) {
    reset(allNotes);
}

ChordNamer::Interval::Interval(const std::vector<Note> &allNotes) : allNotes(allNotes) {
    uniqueIndexes = Note::getUniqueIndexes(allNotes);
}

ChordNamer::Interval &ChordNamer::Interval::reset(const std::string &line) {
    split(line);
    if (allNotes.size() < 2) {
        throw std::length_error("At least two notes are required.");
    }
    uniqueIndexes = Note::getUniqueIndexes(allNotes);
    return *this;
}

ChordNamer::Interval &ChordNamer::Interval::reset(const std::vector<std::string> &allNotes) {
    this->allNotes.clear();
    //convert strings to notes
    for (const std::string &note_str: allNotes) {
        this->allNotes.emplace_back(note_str);
    }
    uniqueIndexes = Note::getUniqueIndexes(this->allNotes);
    return *this;
}

ChordNamer::Interval &ChordNamer::Interval::reset(const std::vector<Note> &allNotes) {
    this->allNotes = allNotes;
    uniqueIndexes = Note::getUniqueIndexes(allNotes);
    return *this;
}

std::vector<std::string>
ChordNamer::Interval::getIntervalList(const std::vector<uint32_t> &distances, const bool chordMode) {
    std::vector<std::string> possible_intervals = {"1", "b9", "9", "b3", "3", "11", "b5", "5", "#5", "6", "b7", "7"};

    bool exist[12] = {false};
    //below compare with semitone interval
    //if 3 and b3 is present, b3 becomes #9
    //if 5 and (b5 || #5) is present, (#11 || b13) is used
    //if 7 is present, 6 becomes 13
    //if b3/3 is NOT present, 4 is used instead of 11
    //if both b3/3 and 4 are NOT present, then 2 is used instead of 9
    //if b3 and b5 and 6 are present (diminished) , then 6/13 becomes 7 and b6/b13 becomes 13

    bool thirdPresent = false;

    for (uint32_t distance: distances) {
        exist[distance] = true;
    }

    if (exist[M3]) {
        //major 3rd
        possible_intervals[m3] = "#9";
        thirdPresent = true;
    }
    if (exist[P5]) {
        //perfect 5th
        possible_intervals[d5] = "#11";
        possible_intervals[m6] = "b13";
    }
    if (exist[m7] || exist[M7]) {
        //minor or major 7th
        possible_intervals[M6] = "13";
    }
    if (exist[m3]) {
        //minor 3rd
        thirdPresent = true;
        if (exist[d5] && exist[d7] && !exist[m7] && !exist[M7]) {
            //full diminished chord
            if (chordMode) {
                possible_intervals[d7] = "7";
                possible_intervals[m6] = "13";
            } else {
                possible_intervals[d7] = "bb7";
                possible_intervals[m6] = "b13";
            }
        }
    }

    if (!thirdPresent) {
        //sus chord
        possible_intervals[P4] = "4";

        if (!exist[P4])
            possible_intervals[M2] = "2";
    }

    return possible_intervals;
}

std::vector<std::string> ChordNamer::Interval::distancesToIntervals(const std::vector<uint32_t> &distances) {
    std::vector<std::string> intervalList = getIntervalList(distances);

    std::vector<std::string> intervals; //actual intervals present in the input list
    intervals.reserve(distances.size());

    for (const uint32_t &semitone: distances) {
        intervals.push_back(intervalList[semitone]);
    }
    return intervals;
}

void ChordNamer::Interval::split(const std::string &line) {
    this->allNotes.clear();
    std::string note_str;
    for (const char c: line) {
        if ((c == ' ' || c == ',')) {
            //encounters delimiter
            if (!note_str.empty()) {
                allNotes.emplace_back(note_str);
                note_str.clear();
            }
        } else {
            note_str += c;
        }
    }
    if (!note_str.empty()) {
        allNotes.emplace_back(note_str);
    }
}
