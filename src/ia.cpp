#include "ia.h"

uint8_t ia_fragment::get_best_position(ia_fragment &fragment) {
	uint8_t better_pos = 0;
	uint8_t better_type = 0;

	for (uint8_t index_pos : this->possibles) {
		if (chess::map[index_pos].type >= better_type) {
			better_type = chess::map[index_pos].type;
		}

		if (fragment.contains(index_pos)) {
			continue;
		}
	}

	return chess::OUT_RANGE;
}

bool ia_fragment::contains(uint8_t pos) {
	for (uint8_t index_pos : this->possibles) {
		if (index_pos == pos) {
			return true;
		}
	}

	return false;
}

void ia_manager::send_mouse_click(int32_t x, int32_t y) {
	SDL_Event sdl_event;

	sdl_event.type = SDL_MOUSEBUTTONDOWN;
	sdl_event.motion.type = SDL_MOUSEBUTTONDOWN;
	sdl_event.motion.x = x;
	sdl_event.motion.y = y;

	SDL_PushEvent(&sdl_event);
}

void ia_manager::init(chess* instance) {
	this->chess_game = instance;
}

void ia_manager::phase_collector() {
	ia_fragment concurrent_fragment;

	/* Start of phase enemy collection. */
	for (entity_piece &entity : chess::loaded_entity_list) {
		if (entity.color_factory == chess::KILLED_TO_ETERNETIY || entity.dead) {
			continue;
		}

		if (entity.color_factory == 0) {
			continue;
		}

		chess::matrix::vec(this->vec, entity.pos);
		chess::matrix::possible(this->concurrent_possible, entity.piece_slot.type, 1, this->vec[0], this->vec[1]);
		
		if (this->concurrent_possible.empty()) {
			continue;
		}

		concurrent_fragment.color = 1;
		concurrent_fragment.pos = entity.pos;
		concurrent_fragment.type = entity.piece_slot.type;
		concurrent_fragment.possibles = this->concurrent_possible;

		// End.
		this->loaded_fragment_enemy.push_back(concurrent_fragment);
		this->concurrent_possible.clear();
	}
	/* End of phase enemy collection. */

	/* Start of phase collection. */
	for (entity_piece &entity : chess::loaded_entity_list) {
		if (entity.color_factory == chess::KILLED_TO_ETERNETIY) {
			continue;
		}

		if (entity.color_factory) {
			continue;
		}

		if (entity.dead) {
			this->loaded_entity_dead.push_back(entity);
			continue;
		}

		chess::matrix::vec(this->vec, entity.pos);
		chess::matrix::possible(this->concurrent_possible, entity.piece_slot.type, 0, this->vec[0], this->vec[1]);

		if (this->concurrent_possible.empty()) {
			continue;
		}

		concurrent_fragment.color = 0;
		concurrent_fragment.pos = entity.pos;
		concurrent_fragment.type = entity.piece_slot.type;
		concurrent_fragment.possibles = this->concurrent_possible;

		// End.
		this->loaded_fragment.push_back(concurrent_fragment);
		this->concurrent_possible.clear();
	}
	/* End of phase collection. */
}

void ia_manager::phase_dispatch() {
	/* Start of phase disaptch. */
	uint8_t better_piece_pos = chess::OUT_RANGE;
	uint8_t better_pos = chess::OUT_RANGE;
	uint8_t better_type = chess::piece::EMPTY;

	bool flag = false;

	for (ia_fragment &black_fragment : this->loaded_fragment) {
		for (ia_fragment &white_fragment : this->loaded_fragment_enemy) {
			if (black_fragment.contains(white_fragment.pos)) {
				flag = true;
				break;
			}
		}

		if (flag) {
			break;
		}
	}

	this->loaded_fragment_result.clear();

	for (ia_fragment &black_fragment : this->loaded_fragment) {
		if (flag) {
			for (ia_fragment &white_fragment : this->loaded_fragment_enemy) {
				if (black_fragment.contains(white_fragment.pos) && chess::map[white_fragment.pos].type >= better_type) {
					better_type = chess::map[white_fragment.pos].type;
					better_piece_pos = black_fragment.pos;
					better_pos = white_fragment.pos;

					util::log("Coletado de algum fragmento.");
				}
			}
			
			for (uint8_t pos : black_fragment.possibles) {
				if (chess::map[pos].type >= better_type) {
					better_type = chess::map[pos].type;
					better_piece_pos = black_fragment.pos;
					better_pos = pos;

					util::log("Coletado de algum fragmento.");
				}
			}

			continue;
		}

		this->loaded_fragment_result.push_back(black_fragment);
	}

	if (!flag) {
		uint8_t count = 0;
		uint8_t stamp = rand() % util::math::clampi(this->loaded_fragment_result.size(), 0, this->loaded_fragment_result.size() - 1);

		uint8_t substamp = 0;
		uint8_t subcount = 0;

		for (ia_fragment &fragment : this->loaded_fragment_result) {
			flag = count >= stamp;

			if (flag) {
				substamp = rand() % util::math::clampi(fragment.possibles.size(), 0, fragment.possibles.size() - 1);
			
				for (uint8_t pos : fragment.possibles) {
					if (subcount >= substamp) {
						better_type = chess::map[fragment.pos].type;
						better_piece_pos = fragment.pos;
						better_pos = pos;
						break;
					}
		
					subcount++;
				}
			}

			if (flag) {
				break;
			}

			count++;
		}
	}

	this->click_start_pos = better_piece_pos;	
	this->click_end_pos = better_pos;

	this->loaded_fragment.clear();
	this->loaded_entity_dead.clear();
	this->loaded_fragment_enemy.clear();
	/* End of pahse dispatch. */
}

void ia_manager::phase_end() {
	/* Start of end. */
	if (this->click_start_pos != chess::OUT_RANGE) {
		util::log("Click inicial.");

		chess::matrix::vec(this->vec, this->click_start_pos);
		this->send_mouse_click(chess_game->x + (this->vec[0] * chess::square_size) + (SQUARE_OFFSET),
							   chess_game->y + (this->vec[1] * chess::square_size) + (SQUARE_OFFSET));
	}

	if (this->click_end_pos != chess::OUT_RANGE) {
		util::log("Click final.");

		chess::matrix::vec(this->vec, this->click_end_pos);
		this->send_mouse_click(chess_game->x + (this->vec[0] * chess::square_size) + (SQUARE_OFFSET),
							   chess_game->y + (this->vec[1] * chess::square_size) + (SQUARE_OFFSET));
	}
	/* End of phase end. */
}

void ia_manager::on_update(uint64_t delta) {
	if (this->chess_game->previous_color_moved == 0) {
		this->interval = false;
	}

	if (this->chess_game->previous_color_moved && !this->interval) {
		this->phase_collector();
		this->phase_dispatch();
		this->phase_end();

		// End.
		this->interval = true;
	}
}