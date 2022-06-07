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

uint8_t ia_manager::get_level(uint8_t type) {
	return this->level[type];
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

	this->loaded_fragment_priority_enemy.clear();
	this->loaded_fragment_enemy.clear();
	this->loaded_fragment.clear();
	this->loaded_fragment_result.clear();
	this->loaded_priority_result.clear();
	this->loaded_fragment_factor.clear();
	this->loaded_entity_dead.clear();
	this->concurrent_possible.clear();
}

float ia_manager::get_val(uint8_t pos, uint8_t color) {
	return color;
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

	this->level[chess::piece::EMPTY]  = 1;
	this->level[chess::piece::PAWN]   = 2;
	this->level[chess::piece::HORSE]  = 7;
	this->level[chess::piece::BISHOP] = 7;
	this->level[chess::piece::KING]   = 10;
	this->level[chess::piece::TOWER]  = 8;
	this->level[chess::piece::QUEEN]  = 9;
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

		if (entity.color_factory || entity.dead) {
			continue;
		}

		if (entity.piece_slot.type == chess::piece::KING) {
			this->king_pos = entity.pos;
		}

		chess::matrix::vec(this->vec, entity.pos);
		chess::matrix::possible(this->concurrent_possible, entity.piece_slot.type, 0, this->vec[0], this->vec[1]);

		if (this->concurrent_possible.empty()) {
			util::log(std::to_string(chess::map[entity.pos].type));

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

	ia_fragment_factor concurrent_fragment_factor;

	for (ia_fragment &fragment : this->loaded_fragment_enemy) {
		chess::matrix::vec(this->vec, fragment.pos);
		chess::matrix::possible(concurrent_fragment.possibles, fragment.type, 1, this->vec[0], this->vec[1], true);

		for (uint8_t pos : fragment.possibles) {
			for (ia_fragment &subfragment : this->loaded_fragment) {
				if (!subfragment.contains(pos)) {
					continue;
				}

				concurrent_fragment_factor.start = fragment.pos;
				concurrent_fragment_factor.end = pos;
				concurrent_fragment_factor.factor = chess::map[pos].type;

				this->loaded_fragment_factor.push_back(concurrent_fragment_factor);

				//util::log("Fragment factor collected() " + std::to_string(this->loaded_fragment_factor.size()));
			}

			if (pos == this->king_pos) {
				hide_king = true;

				concurrent_fragment.pos = fragment.pos;
				concurrent_fragment_killer.possibles.push_back(pos);

				util::log("Possible enemy fragment collected () " + std::to_string(concurrent_fragment_killer.possibles.size()));
			}
		}
	}

	uint8_t concurrent_factor = 0;

	for (ia_fragment_factor &fragment_factor : this->loaded_fragment_factor) {
		chess::matrix::vec(this->vec, fragment_factor.start);
		chess::matrix::possible(this->concurrent_possible, chess::map[fragment_factor.start].type, 0, this->vec[0], this->vec[1]);

		concurrent_factor = 0;

		for (uint8_t pos : this->concurrent_possible) {
			if (this->get_level(chess::map[pos].type) >= concurrent_factor) {
				concurrent_factor = this->get_level(chess::map[pos].type);
				//util::log("Calculating max factor () " + std::to_string(concurrent_factor));
			}
		}

		fragment_factor.factor += concurrent_factor;
		this->concurrent_possible.clear();
	}

	uint8_t better_pos = chess::piece::EMPTY;
	uint8_t better_type = chess::piece::EMPTY;
	uint8_t better_piece = chess::piece::EMPTY;
	uint8_t better_point = 0;

	chess::matrix::vec(this->vec, this->king_pos);
	chess::matrix::possible(this->critc_king_fragment.possibles, chess::piece::KING, 0, this->vec[0], this->vec[1], false);

	bool stop = false;
	bool pass = false;

	for (uint8_t pos : this->critc_king_fragment.possibles) {
		if (pos == this->king_pos_enemy) {
			start_pos = this->king_pos;
			end_pos = this->king_pos_enemy;

			util::log("IA wins using the king!");
			this->dispatch(start_pos, end_pos);

			stop = true;
			break;
		}
	}

	if (stop) {
		return;
	}

	ia_fragment concurrent_collection_fragment;

	for (ia_fragment &fragment : this->loaded_fragment) {
		for (uint8_t pos : fragment.possibles) {
			if (pos == this->king_pos_enemy || chess::map[pos].type == chess::piece::QUEEN) {
				start_pos = fragment.pos;
				better_type = chess::map[pos].type;
				better_pos = pos;
				stop = true;

				util::log("One critic fragment is collected");
				break;
			}

			if ((chess::map[pos].type == fragment.type && chess::map[pos].type >= better_type) || this->get_level(chess::map[pos].type) >= this->get_level(fragment.type)) {
				start_pos = fragment.pos;
				better_type = chess::map[pos].type;
				better_pos = pos;
			}

			if (hide_king && concurrent_fragment.pos == pos) {
				start_pos = fragment.pos;
				better_type = chess::map[concurrent_fragment.pos].type;
				better_pos = pos;
				stop = true;

				util::log("The king is needing help !");
				break;
			}
		}

		if (stop) {
			break;
		}
	}

	if (hide_king && better_pos != concurrent_fragment.pos && !stop) {
		util::log("Start of escapes king !");

		concurrent_fragment.possibles.clear();
		stop = false;

		start_pos = this->king_pos;

		for (uint8_t pos : this->critc_king_fragment.possibles) {
			if (concurrent_fragment_killer.contains(pos)) {
				continue;
			}

			better_pos = pos;

			for (ia_fragment &fragment : this->loaded_fragment_enemy) {
				if (fragment.pos == pos) {
					stop = true;
					break;
				}
			}

			if (stop) {
				break;
			}

			concurrent_fragment.possibles.clear();
		}

		util::log("An escape fragment found !!!");
	}

	pass = false;

	if (!hide_king && better_type == chess::piece::EMPTY && !stop) {
		better_type = chess::piece::EMPTY;
		uint8_t size = chess::piece::EMPTY;

		for (ia_fragment_factor &fragment_factor : this->loaded_fragment_factor) {
			if (size >= fragment_factor.factor) {
				start_pos = fragment_factor.start;
				better_pos = fragment_factor.end;
				size = fragment_factor.factor;
				better_type = chess::map[start_pos].type;

				util::log("Fragment factor max () " + std::to_string(better_type));
			}
		}

		pass = better_type == chess::piece::EMPTY;
	}

	if (pass) {
		uint16_t count = 0;
		uint16_t stamp = rand() % util::math::clampi(this->loaded_fragment.size(), 0, this->loaded_fragment.size() - 1);

		uint16_t substamp = 0;
		uint16_t subcount = 0;

		util::log("Non logic fragment () " + std::to_string(stamp));

		for (ia_fragment &fragment : this->loaded_fragment) {
			flag = count >= stamp;

			if (flag) {
				substamp = rand() % util::math::clampi(fragment.possibles.size(), 0, fragment.possibles.size() - 1);
				util::log("Non sub-fragment logic () " + std::to_string(substamp));
			
				for (uint8_t pos : fragment.possibles) {
					if (chess::map[pos].type == chess::piece::KING || chess::map[pos].type == chess::piece::QUEEN) {
						substamp++;
						flag = false;
						continue;
					}

					if (subcount >= substamp) {
						better_type = chess::map[fragment.pos].type;
						start_pos = fragment.pos;
						better_pos = pos;
						flag = true;
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
	
	end_pos = better_pos;

	if (better_pos == this->king_pos_enemy) {
		util::log("IA gave you a checkmate !!!");
	}

	this->dispatch(start_pos, end_pos);
	/* End of phase dispatch. */
}

void ia_manager::phase_end() {
	/* Start of end. */
	if (this->click_start_pos != chess::OUT_RANGE) {
		util::log("End click pos () " + std::to_string(this->click_start_pos));

		chess::matrix::vec(this->vec, this->click_start_pos);
		this->send_mouse_click(chess_game->x + (this->vec[0] * chess::square_size) + (SQUARE_OFFSET),
							   chess_game->y + (this->vec[1] * chess::square_size) + (SQUARE_OFFSET));
	}

	if (this->click_end_pos != chess::OUT_RANGE) {
		util::log("Start click pos () " + std::to_string(this->click_end_pos));

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

	if (this->chess_game->ressurection && this->chess_game->color_ressure == 0) {
		int32_t pos_x = chess_game->rina_notify.x + SQUARE_OFFSET;
		int32_t pos_y = chess_game->rina_notify.y + SQUARE_OFFSET;

		// Click at queen, the most power full piece.
		this->send_mouse_click(pos_x, pos_y);
	}

	if (this->chess_game->previous_color_moved && !this->chess_game->ressurection && !this->interval) {
		this->phase_collector();
		this->phase_dispatch();
		this->phase_end();

		// End.
		this->interval = true;
	}
}