#pragma once

#include "resource.h"

enum GameState { ReadyToStart, InProgress, Finished };
enum FieldState{CLEAR, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, BOMB};

struct Field
{
	// defines the type of a field. CLEAR - EIGHT mean the field does not contain a bomb
	// CLEAR - no bomb in immediate vicinity
	// ONE - EIGHT - 1 to 8 bombs in vicinity
	// BOMB - there is a bomb buried somewhere in this field

	HWND hWnd = NULL;
	int type = FieldState::CLEAR;
	bool IsClicked = false;
	bool IsFlagged = false;
};