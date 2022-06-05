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
	piece_data piece_slot;
	
	bool dead;
	bool the_god_is_trying_to_talk_with_you;

	float x, y, w, h;
	float previous_x, previous_y;

	uint8_t color_of_tha_death;
	uint8_t color_factory;
	uint8_t pos;

	entity_piece() {}
	entity_piece(float pos_x, float pos_y, float metrics_w, float metrics_h, uint8_t color, piece_data &_piece_data, util::texture &_texture) {
		this->x = pos_x;
		this->y = pos_y;
		this->w = metrics_w;
		this->h = metrics_h;
		this->piece_slot = _piece_data;
	}

	void kill(uint8_t the_death_master);
	void ressure(uint8_t pos);

	void set_color(uint8_t color);
	void set(piece_data &_piece_data);
	void on_render(float render_ticks);
};

struct chess {
	static uint8_t OUT_RANGE;
	static uint8_t TOP;
	static uint8_t BOTTOM;
	static uint8_t KILLED_TO_ETERNETIY;

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
		static void move(uint8_t &not_matrix_pos, int8_t velocity);

		static bool get(piece_data &_piece_data, uint8_t pos);
		static void possible(std::vector<uint8_t> &pos_list, uint8_t type, uint8_t color_factory, uint8_t row, uint8_t col);

		static void align(int8_t* matrix_2x2_1, int8_t* matrix_2x2_2);
		static void unalign(int8_t* matrix_2x2_1, int8_t* matrix_2x2_2);
		static void vec(uint8_t _vec[2], uint8_t pos);
		static uint8_t find(uint8_t row, uint8_t col);
	};

	static piece_data map[64];
	static std::vector<entity_piece> loaded_entity_list;
	static std::map<uint8_t, piece_data> piece_type_map;
	static float square_size;

	static util::texture texture;
	static uint8_t white_dock; 

	static void set_piece(entity_piece &entity, uint8_t type);
	static void move(entity_piece &entity, uint8_t pos);
	static bool entities_bouding_box_collide(entity_piece &entity_1, entity_piece &entity_2);
	static bool relative_height(entity_piece &entity, uint8_t color_factor, float &height, uint8_t i = 8);
	static bool get(entity_piece &entity, uint8_t pos);

	static void crawl_to_the_ressurection(entity_piece &the_entity);
	static void creep_4_tha_death(entity_piece &the_death_as_an_entity_piece, uint8_t &entity_type_killed);

	util::color color_white = util::color(255, 255, 255, 255), color_black = util::color(0, 0, 0, 255);
	std::vector<uint8_t> possible;

	piece_data hovered;
	piece_data start, end;

	uint8_t matrix_pos[4];
	uint8_t focused;
	uint8_t color_ressure;

	entity_piece rina_notify;

	bool start_pos, end_pos;
	bool update;
	bool over;
	bool gaming;
	bool ressurection;

	/* Gamemodes in game. */
	bool gamemode_cycle;
	bool gamemode_godmode;

	uint8_t concurrent_color_moved;
	uint8_t previous_color_moved;

	float x, y, w, h, dx, dy, screen_w, screen_h, my;
	uint8_t alpha;

	void new_game();
	void end_game();
	
	void init(SDL_Window* sdl_window);
	void refresh(SDL_Window* sdl_window = nullptr);
	void set_pos(float pos_x, float pos_y);
	bool contains(uint8_t pos);

	void on_event(SDL_Event &sdl_event);
	void on_update(uint64_t delta);
	void on_render(float render_ticks);
};

#endif