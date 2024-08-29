#include <cstdio>
#include <string>
#include <vector>
#include <iostream>

#include "chord.h"

using namespace ChordNamer;

int main() {
	std::string line;
	Chord chord;

	while (1) {
		puts("Enter notes separated by spaces or commas:");
		std::getline(std::cin, line);
		printf("Entered notes: %s\n", line.c_str());
		try {
			chord.reset(line);
			for (const std::string &chordName: chord.chordNames) {
				printf("Chord: %s\n", chordName.c_str());
			}
			printf("\n");
		} catch (const char *msg) {
			printf("\n%s\n\n", msg);
		}
	}

	return 0;
}
