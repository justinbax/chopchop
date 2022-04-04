#include "game.h"

Game::Game() {
	this->current[0] = this->current[1] = this->next[0] = this->next[1] = 0;
	this->simulated = false;
	this->evaluated = false;
	this->evaluation = 0.0f;
}

Game::Game(int current1, int current2, int next1, int next2) {
	this->current[0] = current1;
	this->current[1] = current2;
	this->next[0] = next1;
	this->next[1] = next2;

	this->simulated = false;
	this->evaluated = false;
	this->evaluation = 0.0f;
}

bool Game::operator==(Game other) {
	if (this->current[0] == other.current[0] && this->current[1] == other.current[1] && this->next[0] == other.next[0] && this->next[1] == other.next[1])
		return true;
	return false;
}

void Game::order() {
	if (this->current[1] > this->current[0]) {
		int old = this->current[0];
		this->current[0] = this->current[1];
		this->current[1] = old;
	}

	if (this->next[1] > this->next[0]) {
		int old = this->next[0];
		this->next[0] = this->next[1];
		this->next[1] = old;
	}
}

Game Game::turn() const {
	Game newGame;
	newGame.current[0] = this->next[0];
	newGame.current[1] = this->next[1];
	newGame.next[0] = this->current[0];
	newGame.next[1] = this->current[1];
	return newGame;
}

Game Game::attack(int add1, int add2) const {
	Game newGame(*this);

	newGame.next[0] += add1;
	newGame.next[1] += add2;

	if (newGame.next[1] > MAXHAND) {
		newGame.next[1] = 0;
	}

	if (newGame.next[0] > MAXHAND) {
		newGame.next[0] = 0;
	}

	return newGame;
}

Game Game::transfer(int count) const {
	Game newGame(*this);

	// Someone said to me it should be count <= newGame.current[0] instead of < so you can do 2111 2t3 1130.
	// Now the computer thinks 1111 is losing -100.0f and there are only 9 non-winning or non-losing positions.
	// I am desperately trying to beat it but I think it has become sentient.
	// Send help.

	if (count > 0 && count <= newGame.current[0] && (newGame.current[1] + count) <= MAXHAND) {
		newGame.current[0] -= count;
		newGame.current[1] += count;
	} else if (count < 0 && (newGame.current[0] - count) <= MAXHAND && newGame.current[1] >= -count) {
		newGame.current[0] -= count;
		newGame.current[1] += count;
	}

	return newGame;
}