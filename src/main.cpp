#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>

#include "interval.h"
#include "chord.h"

using std::string;
using std::vector;

#ifdef _WIN32
#define clearScreen "cls" //in Windows
#else
#define clearScreen "clear" //in any other OS
#endif

#pragma warning(disable: 4996)

int main(void) {

	string line;
	chord theChord;
	
	while (1) {
		puts("Enter notes separated by spaces or commas:");
		std::getline(std::cin, line);
		system(clearScreen);
		printf("Entered notes: %s\n\n", line.c_str());
		try {
			theChord.reset(line).printAllPossible();
		} catch (const char *msg) {
			printf("\n%s\n", msg);
		}
	}

	return 0;
}