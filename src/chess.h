#pragma once

#include "includes.h"
#include "util.h"

#ifndef CHESS_H
#define CHESS_H

struct piece_data {
	float x, y, w, h;

	uint8_t type;
	uint8_t color;
	uint8_t moved;

	void set(float rect_x, float rect_y, float rect_w, float rect_g);
};

struct entity_piece {
	float x, y, w, h;
	float previous_x, previous_y;

	bool alive;
	uint8_t death_master;
	uint8_t color_factory;
	uint8_t pos;

	piece_data piece_slot;
	util::texture texture;

	entity_piece() {}
	entity_piece(float pos_x, float pos_y, float metrics_w, float metrics_h, uint8_t color, piece_data &_piece_data, util::texture &_texture) {
		this->x = pos_x;
		this->y = pos_y;
		this->w = metrics_w;
		this->h = metrics_h;
		this->piece_slot = _piece_data;
		this->texture = _texture;
	}

	void set_color(uint8_t color);
	void set(piece_data &_piece_data);
	void on_render(float render_ticks);
};

struct chess {
	static uint8_t OUT_RANGE;
	static uint8_t TOP;
	static uint8_t DOWN;

	struct piece {
		static uint8_t PAWN;
		static uint8_t TOWER;
		static uint8_t HORSE;
		static uint8_t BISHOP;
		static uint8_t QUEEN;
		static uint8_t KING;
		static uint8_t EMPTY;
	};

	enum color {
		WHITE, BLACK
	};

	struct render {
		static piece_data pawn;
		static piece_data tower;
		static piece_data horse;
		static piece_data bishop;
		static piece_data queen;
		static piece_data king;

		static void init();
		static chess::color get_color(entity_piece &entity);
		static void set_color(entity_piece &entity, chess::color piece_color);
	};

	struct matrix {
		static void from(float x, float y, uint8_t &row, uint8_t &col);
		static void move(uint8_t &not_matrix_pos, uint8_t velocity);

		static bool get(piece_data &_piece_data, uint8_t pos);
		static void possible(std::vector<uint8_t> &pos_list, uint8_t type, uint8_t color_factory, uint8_t row, uint8_t col);

		static void vec(uint8_t _vec[2], uint8_t pos);
		static uint8_t find(uint8_t row, uint8_t col);
	};

	static piece_data map[64];
	static std::vector<entity_piece> loaded_entity_list;
	static std::map<uint8_t, piece_data> piece_type_map;
	static float square_size;

	std::vector<uint8_t> possible;

	piece_data hovered;
	piece_data start, end;

	uint8_t matrix_pos[4];
	uint8_t focused;

	bool start_pos, end_pos;

	bool over;
	bool gaming;
	bool dragging;

	float x, y, w, h, dx, dy, screen_w, screen_h;
	uint8_t alpha;
	
	static util::texture texture;
	static uint8_t white_dock; 

	util::color color_white = util::color(255, 255, 255, 255), color_black = util::color(0, 0, 0, 255);

	static void set_piece(entity_piece &entity, uint8_t type);
	static uint8_t find(const std::string &letter, uint8_t slot);
	static void move(entity_piece &entity, uint8_t pos);

	void new_game();
	void end_game();
	
	void init();
	void refresh();
	bool contains(uint8_t pos);

	void on_event(SDL_Event &sdl_event);
	void on_render(float render_ticks);
};

#endif