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

void ia_fragment::get_level(uint8_t type) {

}

bool ia_fragment::contains(uint8_t pos) {
	for (uint8_t index_pos : this->possibles) {
		if (index_pos == pos) {
			return true;
		}
	}

	return false;
}

void ia_manager::dispatch(uint8_t start_pos, uint8_t end_pos) {
	this->click_start_pos = start_pos;	
	this->click_end_pos = end_pos;

	this->loaded_fragment.clear();
	this->loaded_entity_dead.clear();
	this->loaded_fragment_enemy.clear();
	this->critc_king_fragment.possibles.clear();
	this->concurrent_possible.clear();
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
	std::srand(time(0));

	this->level[chess::piece::PAWN] = 1;
	this->level[chess::piece::HORSE] = 3;
	this->level[chess::piece::BISHOP] = 3;
	this->level[chess::piece::KING] = 10;
	this->level[chess::piece::TOWER] = 5;
	this->level[chess::piece::QUEEN] = 9;
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

		if (entity.piece_slot.type == chess::piece::KING) {
			this->king_pos_enemy = entity.pos;
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

		if (entity.piece_slot.type == chess::piece::KING) {
			this->king_pos = entity.pos;
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
	bool hide_king = false;
	bool flag = false;

	ia_fragment concurrent_fragment;
	ia_fragment concurrent_fragment_killer;

	uint8_t start_pos = 0;
	uint8_t end_pos = 0;

	for (ia_fragment &fragment : this->loaded_fragment_enemy) {
		chess::matrix::vec(this->vec, fragment.pos);
		chess::matrix::possible(concurrent_fragment.possibles, fragment.type, 1, this->vec[0], this->vec[1], true);

		for (uint8_t pos : fragment.possibles) {
			flag = fragment.type == chess::piece::QUEEN || fragment.type == chess::piece::TOWER || fragment.type == chess::piece::HORSE || fragment.type == chess::piece::BISHOP;

			if (flag && (concurrent_fragment.contains(pos) && fragment.type == chess::piece::HORSE)) {
				hide_king = true;
				concurrent_fragment_killer.possibles.push_back(pos);
			}
		}
	}

	chess::matrix::vec(this->vec, this->king_pos);
	chess::matrix::possible(this->critc_king_fragment.possibles, chess::piece::KING, 0, this->vec[0], this->vec[1], true);

	for (uint8_t pos : this->critc_king_fragment.possibles) {
		if (pos == this->king_pos_enemy) {
			start_pos = this->king_pos;
			end_pos = this->king_pos_enemy;

			break;
		}
	}

	uint8_t better_pos = chess::piece::EMPTY;
	uint8_t better_type = chess::piece::EMPTY;

	for (ia_fragment &fragment : this->loaded_fragment) {
		for (uint8_t pos : fragment.possibles) {
			if (chess::map[pos].type >= better_type) {
				better_type = chess::map[pos].type;
				start_pos = fragment.pos;
				better_pos = pos;

				util::log(std::to_string(chess::map[pos].type));

				this->loaded_fragment_result.push_back(fragment);
			}

			if (pos == this->king_pos_enemy) {
				start_pos = fragment.pos;
				end_pos = this->king_pos_enemy;
				break;
			}

			if (hide_king && concurrent_fragment_killer.contains(pos)) {
				start_pos = fragment.pos;
				better_pos = pos;
				break;
			}
		}
	}

	if (!hide_king && better_type == chess::piece::EMPTY) {
		uint16_t count = 0;
		uint16_t stamp = rand() % util::math::clampi(this->loaded_fragment_result.size(), 0, this->loaded_fragment_result.size() - 1);

		uint16_t substamp = 0;
		uint16_t subcount = 0;

		util::log("Rand, " + std::to_string(stamp));

		for (ia_fragment &fragment : this->loaded_fragment_result) {
			flag = count >= stamp;

			if (flag) {
				substamp = rand() % util::math::clampi(fragment.possibles.size(), 0, fragment.possibles.size() - 1);
			
				for (uint8_t pos : fragment.possibles) {
					if (subcount >= substamp) {
						better_type = chess::map[fragment.pos].type;
						start_pos = fragment.pos;
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

	if (end_pos != this->king_pos_enemy) {
		end_pos = better_pos;
	}

	this->dispatch(start_pos, end_pos);
	/* End of phase dispatch. */
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

	if (this->chess_game->previous_color_moved && !this->chess_game->ressurection && !this->interval) {
		this->phase_collector();
		this->phase_dispatch();
		this->phase_end();

		// End.
		this->interval = true;
	}
}