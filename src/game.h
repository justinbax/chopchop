#ifndef GAME_H
#define GAME_H

#include <vector>

#define MAXHAND 4

struct Game {
	int current[2];
	int next[2];
	bool simulated;
	bool evaluated;
	float evaluation;

	std::vector<Game *> parents;
	std::vector<Game *> children;

	Game();
	Game(int current1, int current2, int next1, int next2);
	bool operator==(Game other);

	void order();
	Game turn() const;
	Game attack(int add1, int add2) const;
	Game transfer(int count) const;
};

#endif // ifndef GAME_H