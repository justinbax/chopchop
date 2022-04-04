#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>

#include <math.h>
#include <iomanip>

#include "game.h"

#define EVALUATIONCOUNT 5000
#define NULLGAME Game(-1, -1, -1, -1)

// Returns a pointer to the situation given in the games array
Game *findGame(Game game, Game games[MAXHAND + 1][MAXHAND + 1][MAXHAND + 1][MAXHAND + 1]) {
	return &(games[game.current[0]][game.current[1]][game.next[0]][game.next[1]]);
}

void iterateGames(Game games[MAXHAND + 1][MAXHAND + 1][MAXHAND + 1][MAXHAND + 1], void (*f)(Game *)) {
	for (int i = 0; i < MAXHAND + 1; i++) {
		for (int j = 0; j < MAXHAND + 1; j++) {
			for (int k = 0; k < MAXHAND + 1; k++) {
				for (int l = 0; l < MAXHAND + 1; l++) {
					f(&games[i][j][k][l]);
				}
			}
		}
	}
}

bool simulate(Game game, Game parentGame, Game games[MAXHAND + 1][MAXHAND + 1][MAXHAND + 1][MAXHAND + 1]) {
	game.order();

	Game *gamePtr = findGame(game, games);

	if (!(parentGame == NULLGAME)) {
		// Checks for change between parent game and current game
		Game *parentPtr = findGame(parentGame, games);
		if (parentGame.turn() == game) return false;

		// Else, current simulation is valid and we can update current game's parents and parent game's children
		if (std::find(gamePtr->parents.begin(), gamePtr->parents.end(), parentPtr) == gamePtr->parents.end())
			gamePtr->parents.push_back(parentPtr);

		if (std::find(parentPtr->children.begin(), parentPtr->children.end(), gamePtr) == parentPtr->children.end())
			parentPtr->children.push_back(gamePtr);
	}

	// Or if the game was already simulated
	if (gamePtr->simulated) return true;

	// Game was not simulated: initialize values
	gamePtr->simulated = true;
	gamePtr->current[0] = game.current[0];
	gamePtr->current[1] = game.current[1];
	gamePtr->next[0] = game.next[0];
	gamePtr->next[1] = game.next[1];

	// Or if current lost
	if (game.current[0] == 0 && game.current[1] == 0) return true;

	// Attacks first with both hands
	if (game.next[0] != 0) {
		simulate(game.attack(game.current[0], 0).turn(), game, games);
		simulate(game.attack(game.current[1], 0).turn(), game, games);
	}
	
	// Then second
	if (game.next[1] != 0) {
		simulate(game.attack(0, game.current[0]).turn(), game, games);
		simulate(game.attack(0, game.current[1]).turn(), game, games);
	}

	// Transfer both sides
	for (int i = 1; i < MAXHAND; i++) {
		simulate(game.transfer(i).turn(), game, games);
		simulate(game.transfer(-i).turn(), game, games);
	}

	return true;
}

float evaluate(Game game, Game games[MAXHAND + 1][MAXHAND + 1][MAXHAND + 1][MAXHAND + 1]) {
	Game *gamePtr = findGame(game, games);
	if (!gamePtr->evaluated) {

		gamePtr->evaluated = true;

		// So there's this problem with recursive evaluation:
		// Let A lead to B, which leads to 100.0 and C, which leads to A again.

		//       |-> 100.0
		// A -> B -> C
		// ^---------|

		// A is evaluated first, so calls eval on B. B calls eval on 100.0 and gets 100.0, so adds 50.0 to its own eval.
		// Now B calls eval to its second child, C. C calls eval on A, which didn't recieve the eval of B yet, so it's still 0.0.
		// However, to prevent infinite stack overflow, we can't just evaluate A again and repeat the process, so C must take A's current 0.0 eval.
		// So C gets an eval of 0.0, which doesn't increase B's score of 50.0. So A and B are 50.0 with C being 0.0, even though the only way out is 100.0.
		// Similar problems arise with other recursive situations.
		
		// Right now, these problems are "solved" by repeating evaluation:
		// Once we evaluate games[1][1][1][1] once (equivalent to evaluating every game once), simply do it agin an arbitrary number of times.
		// We keep the approximate evaluation we have while evaluating children, so when recursively evaluating parents, they have a better guess than 0.0.

		// First technique
		// Weighted average :
		// With m total solutions, the nth best is multiplied by
		// 2(m - n) / (m(m + 1))

		// This is to make the weight of a solution linear to its position on best evalution.
		// We want the last term to account for 1 part, the second to last for 2 parts, the third to last for 3 parts, all the way through the best solution accounting for m parts.
		// So the nth best term weighs (m - n) parts (n = 0 is the best solution, n = (m-1) is the worst)

		// We then need to divide by the total number of parts. There are (1 + 2 + 3 + ... + m) parts, which equals m(m + 1) / 2, because
		// 1    2    3   ...  (m/2)
		// +    +    +   ...  +
		// m    m-1  m-2 ...  (m/2) + 1
		// =    =    =        =
		// m+1  m+1  m+1      m+1   } (m/2) times

		// So the total is (m/2) * (m+1) == m(m + 1) / 2.
		// If we divide the number of parts for the nth term (m - n) by the total number of parts (m(m + 1)) / 2, we get
		// (m - n) / ((m(m + 1)) / 2) == 2(m - n) / (m(m + 1))
		// In the end, it also is multiplied by -1 because a negative evaluation for the next player is a positive evaluation of the current position.

		// 1 game  -> 1/1
		// 2 games -> 2/3 , 1/3
		// 3 games -> 3/6 , 2/6 , 1/6
		// 4 games -> 4/10, 3/10, 2/10, 1/10
		// 5 games -> 5/15, 4/15, 3/15, 2/15, 1/15

		// Second technique
		// Standard dispersion ripoff
		// Ep = sum(En * sqrt(En - Em + 1)) / sum(sqrt(En - Em + 1)) where
		// Ep is the evaluation of the current position;
		// En is the evaluation of the nth child of current position;
		// m is the number of children of p;
		// Em is the evaluation of the mth (last) child of p.

		// We basically scale the weight of each child based on how better it is compared to the worst child (more specifically, we scale it by the square root of the distance)
		// The original formula included the mean, but turns out I'm stupid and everything cancels out (basically, we took the distance from the mean, but added a constant
		// to all evaluations so the worst child was at a distance of 1 from the mean, which implied subtracting then adding the mean)
		// The weight is sqrt(Distance from last child's eval + 1) == sqrt(Current child's eval - Last child's eval + 1)
		// We add 1 to the whole thing so the weight isn't 0 for the last child: if each child has the same evaluation, they would all get 0 weight, resulting in an evaluation of 0.
		// Afterwards, you just need to divide by the total amount of weight you gave, so the sum of each square root.

		const int m = gamePtr->children.size();
		if (m == 0) return (gamePtr->evaluation = 0.0f); // Otherwise could mess with other things

		for (int i = 0; i < m; i++) {
			evaluate(*(gamePtr->children[i]), games);

			// Between each child, we temporarily update the evaluation as if it was an average of the evaluation of every previous child.
			// This way, when recursing to the current game, the guess is slightly better than 0.0.
			// Note: while EVALUATIONCOUNT is greater than 1, this cannot be used.
			//gamePtr->evaluation *= (i / (i + 1));
			//gamePtr->evaluation += -1 * result / (i + 1);
		
		}

		// Sort by evaluation, ascending (the worst evaluation for the next player is the best for the current player)
		std::sort(gamePtr->children.begin(), gamePtr->children.end(), 
		[] (Game *a, Game *b) -> bool {
			return a->evaluation < b->evaluation;
		});

		// One child of -100.0 is a guaranteed win; all children being 100.0 is a guaranteed loss.
		// There's something wrong here for sure because 1111 is now losing -100.0f.
		if (abs(gamePtr->children[0]->evaluation) == 100.0f) {
			gamePtr->evaluation = -1 * gamePtr->children[0]->evaluation;
			return gamePtr->evaluation;
		}

		// Dump the temporary score and calculate based on weighted average (best moves weigh more)
		gamePtr->evaluation = 0.0f;

		// Big math
		float totalWeight = 0.0f;

		for (int n = 0; n < m; n++) {
			gamePtr->evaluation += gamePtr->children[n]->evaluation * -2 * (m - n) / (m * (m + 1));

			// TODO a negative evaluation for the next player is a positive evaluation for the current player, so deal with this in the standard dispersion ripoff technique.
			//float weight = sqrt(gamePtr->children[m - 1]->evaluation - gamePtr->children[n]->evaluation + 1);
			//gamePtr->evaluation += gamePtr->children[n]->evaluation * weight;
			//totalWeight += weight;
		}

		//gamePtr->evaluation /= totalWeight;
	}

	return gamePtr->evaluation;
}

int main(int argc, char *argv[]) {
	Game input;

	if (argc == 2 && strlen(argv[1]) == 4) {
		input.current[0] = argv[1][0] - '0';
		input.current[1] = argv[1][1] - '0';
		input.next[0] = argv[1][2] - '0';
		input.next[1] = argv[1][3] - '0';
	} else if (argc == 5) {
		input.current[0] = std::stoi(argv[1]);
		input.current[1] = std::stoi(argv[2]);
		input.next[0] = std::stoi(argv[3]);
		input.next[1] = std::stoi(argv[4]);
	} else if (argc != 1) {
		std::cout << "Usage : game {abcd|a b c d}" << std::endl;
		return 1;
	}

	input.order();

	Game games[MAXHAND + 1][MAXHAND + 1][MAXHAND + 1][MAXHAND + 1];
	Game *initial = &games[1][1][1][1];
	*initial = Game(1, 1, 1, 1);

	simulate(*initial, NULLGAME, games);

	for (int i = 0; i < EVALUATIONCOUNT; i++) {

		iterateGames(games,
		[] (Game *game) -> void {
			game->evaluated = false;
		});

		for (int i = 0; i < MAXHAND + 1; i++) {
			for (int j = 0; j < MAXHAND + 1; j++) {
				games[0][0][i][j].evaluation = -100.0f;
				games[i][j][0][0].evaluation = 100.0f;
				games[0][0][i][j].evaluated = true;
				games[i][j][0][0].evaluated = true;
			}
		}

		evaluate(games[1][1][1][1], games);
	}

	if (argc == 1) {

		std::cout << std::fixed << std::setprecision(4);

		// Output all positions
		iterateGames(games,
		[] (Game *game) -> void {

			if (game->simulated) {
				std::cout << game->current[0] << game->current[1] << game->next[0] << game->next[1] << " : " << (game->evaluation < 0 ? "-" : "+") << std::setw(9) << std::fabs(game->evaluation);
				
				if (!game->children.empty()) {
					Game *best = game->children[0];
					for (int i = 1; i < game->children.size(); i++) {
						if (game->children[i]->evaluation < best->evaluation)
							best = game->children[i];
					}

					std::cout << " (" << best->current[0] << best->current[1] << best->next[0] << best->next[1] << " : " << (best->evaluation < 0 ? "-" : "+") << std::setw(9) << std::fabs(best->evaluation) << ")";
				}
				std::cout << std::endl;
			}
		});

	} else {
		// Output a single position

		Game *inputPtr = findGame(input, games);
		if (!inputPtr->simulated) {
			return 1;
		}

		std::cout << "evaluation : " << inputPtr->evaluation << std::endl << std::endl;

		std::cout << inputPtr->children.size() << " children :" << std::endl;
		for (int i = 0; i < inputPtr->children.size(); i++) {
			Game *child = inputPtr->children[i];
			std::cout << child->current[0] << " " << child->current[1] << "  " << child->next[0] << " " << child->next[1] << " (evaluation : " << child->evaluation << ", " << child->children.size() << " children)" << std::endl;
		}

		std::cout << std::endl << inputPtr->parents.size() << " parents :" << std::endl;
		for (int i = 0; i < inputPtr->parents.size(); i++) {
			Game *parent = inputPtr->parents[i];
			std::cout << parent->current[0] << " " << parent->current[1] << "  " << parent->next[0] << " " << parent->next[1] << " (evaluation : " << parent->evaluation << ", " << parent->children.size() << " children)" << std::endl;
		}
	}

	return 0;
}