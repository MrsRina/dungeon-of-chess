#pragma once

#include "chess.h"
#include "includes.h"

#ifndef IA_H
#define IA_H

static const float SQUARE_OFFSET = 10.0f;

struct ia_fragment {
	uint8_t pos;
	uint8_t type;
	uint8_t color;

	std::vector<uint8_t> possibles;

	bool contains(uint8_t pos);
	uint8_t get_best_position(ia_fragment &fragment);
};

class ia_manager {
protected:
	uint8_t level[6];

	std::vector<ia_fragment> loaded_fragment_priority_enemy;
	std::vector<ia_fragment> loaded_fragment_enemy;
	std::vector<ia_fragment> loaded_fragment;

	std::vector<ia_fragment> loaded_fragment_result;
	std::vector<ia_fragment> loaded_priority_result;

	std::vector<entity_piece> loaded_entity_dead;
	std::vector<uint8_t> concurrent_possible;

	chess* chess_game;
	bool interval;

	uint8_t vec[2];
	uint8_t click_start_pos;
	uint8_t click_end_pos;

	uint8_t king_pos_enemy;
	uint8_t king_pos;

	ia_fragment critc_king_fragment;

	void dispatch(uint8_t start_pos, uint8_t end_pos);
	void send_mouse_click(int32_t x, int32_t y);

	uint8_t get_level(uint8_t type);
	float get_val(uint8_t pos, uint8_t color);
public:
	void phase_collector();
	void phase_dispatch();
	void phase_end();

	void init(chess* instance);
	void on_update(uint64_t delta);
};

#endif