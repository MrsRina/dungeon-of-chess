#include "chess.h"
#include "tessellator.h"
#include "shader.h"

util::texture chess::texture = util::texture();
uint8_t chess::white_dock    = chess::TOP;
float chess::square_size     = 30;

piece_data chess::render::queen  = piece_data();
piece_data chess::render::bishop = piece_data();
piece_data chess::render::pawn   = piece_data();
piece_data chess::render::tower  = piece_data();
piece_data chess::render::horse  = piece_data();
piece_data chess::render::king   = piece_data();

uint8_t chess::OUT_RANGE           = 66;
uint8_t chess::TOP                 = 67;
uint8_t chess::BOTTOM              = 68;
uint8_t chess::KILLED_TO_ETERNETIY = 9;

uint8_t chess::piece::EMPTY  = 0;
uint8_t chess::piece::PAWN   = 1;
uint8_t chess::piece::TOWER  = 2;
uint8_t chess::piece::HORSE  = 3;
uint8_t chess::piece::BISHOP = 4;
uint8_t chess::piece::QUEEN  = 5;
uint8_t chess::piece::KING   = 6;

piece_data chess::map[64] = {};
std::vector<entity_piece> chess::loaded_entity_list = {};
std::map<uint8_t, piece_data> chess::piece_type_map = {};

void piece_data::set(float rect_x, float rect_y, float rect_w, float rect_h) {
	this->x = rect_x;
	this->y = rect_y;
	this->w = rect_w;
	this->h = rect_h;
}

void entity_piece::set_color(uint8_t color) {
	this->color_factory = color;
	this->piece_slot.color = color;
}

void entity_piece::kill(uint8_t color) {
	this->color_of_tha_death = color;
	this->dead = true;
}

void entity_piece::ressure(uint8_t pos) {
	this->pos = pos;
	this->dead = false;
}

void entity_piece::set(piece_data &_piece_data) {
	this->piece_slot = _piece_data;
	this->dead = false;
}

void entity_piece::on_render(float render_ticks) {
	this->previous_x = util::math::lerpf(this->previous_x, this->x, render_ticks);
	this->previous_y = util::math::lerpf(this->previous_y, this->y, render_ticks);

	// Draw the piece.
	util::render::shape_texture(this->previous_x, this->previous_y, this->w, this->h, this->piece_slot.x, this->piece_slot.y, this->piece_slot.w, this->piece_slot.h, chess::texture);
}

void chess::render::init() {
	float width = chess::texture.width / 6;
	float height = chess::texture.height / 2;

	queen.set(0.0f, 0.0F, width, height);
	queen.type = chess::piece::QUEEN;
	chess::piece_type_map[chess::piece::QUEEN] = queen;

	king.set(width * 1, 0.0F, width, height);
	king.type = chess::piece::KING;
	chess::piece_type_map[chess::piece::KING] = king;

	tower.set(width * 2, 0.0F, width, height);
	tower.type = chess::piece::TOWER;
	chess::piece_type_map[chess::piece::TOWER] = tower;

	horse.set(width * 3, 0.0F, width, height);
	horse.type = chess::piece::HORSE;
	chess::piece_type_map[chess::piece::HORSE] = horse;

	bishop.set(width * 4, 0.0F, width, height);
	bishop.type = chess::piece::BISHOP;
	chess::piece_type_map[chess::piece::BISHOP] = bishop;

	pawn.set(width * 5, 0.0F, width, height);
	pawn.type = chess::piece::PAWN;
	chess::piece_type_map[chess::piece::PAWN] = pawn;
}

chess::color chess::render::get_color(entity_piece &entity) {
	return entity.color_factory == 1 ? chess::color::WHITE : chess::color::BLACK;
}

void chess::render::set_color(entity_piece &entity, chess::color piece_color) {
	float height = chess::texture.height / 2;
	entity.piece_slot.y = piece_color == chess::color::WHITE ? height : 0.0f;
}

void chess::matrix::from(float x, float y, uint8_t &row, uint8_t &col) {
	row = (uint8_t) (x / chess::square_size);
	col = (uint8_t) (y / chess::square_size);

	row = util::math::clampi(row, 0, 8); 
	col = util::math::clampi(col, 0, 8); 
}

void chess::matrix::move(uint8_t &not_matrix_pos, int8_t velocity) {
	not_matrix_pos += velocity;
	not_matrix_pos = util::math::clampi(not_matrix_pos, 0, 8);
}

bool chess::matrix::get(piece_data &_piece_data, uint8_t pos) {
	if (pos > 64) {
		return false;
	}

	_piece_data = chess::map[pos];

	for (entity_piece &entity : chess::loaded_entity_list) {
		if (entity.pos == pos && !entity.dead) {
			_piece_data = entity.piece_slot;
			_piece_data.color = entity.color_factory;
			break;
		}
	}

	return true;
}

void chess::matrix::align(int8_t* matrix_2x2_1, int8_t* matrix_2x2_2) {
	// left-down
	matrix_2x2_1[0] = 1;
	matrix_2x2_2[0] = 1;

	// right-down
	matrix_2x2_1[1] = 1;
	matrix_2x2_2[1] = 0;

	// top-right
	matrix_2x2_1[2] = -1;
	matrix_2x2_2[2] = 1;

	// top-left
	matrix_2x2_1[3] = -1;
	matrix_2x2_2[3] = 0;
}

void chess::matrix::unalign(int8_t* matrix_2x2_1, int8_t* matrix_2x2_2) {
	// left-down
	matrix_2x2_1[0] = 1;
	matrix_2x2_2[0] = 1;

	// right-down
	matrix_2x2_1[1] = -1;
	matrix_2x2_2[1] = 1;

	// top-right
	matrix_2x2_1[2] = -1;
	matrix_2x2_2[2] = -1;

	// top-left
	matrix_2x2_1[3] = 1;
	matrix_2x2_2[3] = -1;
}

void chess::matrix::possible(std::vector<uint8_t> &pos_list, uint8_t type, uint8_t color_factory, uint8_t row, uint8_t col) {
	bool white = ((bool) color_factory);
	bool real = false;

	piece_data concurrent_piece;

	uint8_t pos = 0;
	uint8_t next_row = row;
	uint8_t next_col = col;

	// Mask inital used to mark vectors direction and matrix pos.
	int8_t mask_factor_flag_1[4]; // Force.
	int8_t mask_factor_flag_2[4]; // Velocity.

	chess::matrix::align(mask_factor_flag_1, mask_factor_flag_2);

	// I made EVERYTHING in this game-project, except the texture. 
	// The path finder use matrix.

	if (type == chess::piece::PAWN) {
		/* Start of pawn path finder. */
		// Velocity based on side positions.
		int8_t v_white = chess::white_dock == chess::TOP ?  1 : -1;
		int8_t v_black = chess::white_dock == chess::TOP ? -1 :  1;

		chess::matrix::get(concurrent_piece, chess::matrix::find(next_row, next_col));
		chess::matrix::move(next_col, white ? v_white : v_black);

		// Verify if is moved for first time.
		if (concurrent_piece.type == chess::piece::PAWN && !concurrent_piece.moved) {
			real = chess::matrix::get(concurrent_piece, chess::matrix::find(next_row, next_col)) && concurrent_piece.type == chess::piece::EMPTY;
			chess::matrix::move(next_col, white ? v_white : v_black);

			if (real && chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && concurrent_piece.type == chess::piece::EMPTY) {
				pos_list.push_back(pos);
			}
					
			// Reset and move to front.
			next_col = col;
			chess::matrix::move(next_col, white ? v_white : v_black);
		}

		// If we can move front and there is not piece there, yes we add it in pos list.
		if (real = chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && concurrent_piece.type == chess::piece::EMPTY) {
			pos_list.push_back(pos);
		}

		// Move right.
		chess::matrix::move(next_row, 1);

		// If some piece is in up-right to piece, we can kill so add in pos list.
		if (real = chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory) {
			pos_list.push_back(pos);
		}

		// Now move left (considering it was right, so double move to left).	
		chess::matrix::move(next_row, -2);

		// Samething but now for up-left
		if (real = chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory) {
			pos_list.push_back(pos);
		}
		/* End of pawn path finder. */
	} else if (type == chess::piece::TOWER) {
		/* Start of tower path finder. */
		chess::matrix::get(concurrent_piece, chess::matrix::find(next_row, next_col));

		// Concurrent iterations from difference and stages.
		uint8_t concurrent_mask_i = 0;
		uint8_t concurrent_i = 0;
		uint8_t previous_i = 0;

		// 1..33 = 32
		for (uint8_t i = 1; i < 33; i++) {
			concurrent_i = i - previous_i;

			// Move with base in stage (direction)
			chess::matrix::move(mask_factor_flag_2[concurrent_mask_i] == 1 ? next_col : next_row, mask_factor_flag_1[concurrent_mask_i]);

			// Verify if contains an empty slot or if is an enemy.
			real = chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && (concurrent_piece.type == chess::piece::EMPTY || (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory));	
			
			if (real) {
				pos_list.push_back(pos);

				// If is an enemy stop this stage and go next.
				if (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory) {
					real = false;
				}
			}

			// For move the stages.
			if (concurrent_i == 8 || !real) {
				previous_i = i;

				// Break at four stage.
				if (concurrent_mask_i == 3) {
					break;
				}

				concurrent_mask_i++;

				next_row = row;
				next_col = col;
			}
		}
		/* End of tower path finder. */
	} else if (type == chess::piece::HORSE) {
		/* Start of horse path finder. */
		// Concurrent iterations from difference and stages.
		uint8_t concurrent_stage = 0;
		uint8_t concurrent_mask_i = 0;
		uint8_t concurrent_i = 0;
		uint8_t previous_i = 0;

		// 1..13 = 12
		for (uint8_t i = 1; i < 13; i++) {
			concurrent_i = i - previous_i;
			real = true;

			// Move with base in stage (direction)
			chess::matrix::move(mask_factor_flag_2[concurrent_mask_i] == 1 ? next_col : next_row, mask_factor_flag_1[concurrent_mask_i]);
			concurrent_stage++;

			// Now just find some enemy close.
			if (concurrent_stage == 2) {
				chess::matrix::move(mask_factor_flag_2[concurrent_mask_i] == 1 ? next_row : next_col, -1);

				// First side.
				if (real = chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && (concurrent_piece.type == chess::piece::EMPTY || (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory))) {
					pos_list.push_back(pos);
				}

				chess::matrix::move(mask_factor_flag_2[concurrent_mask_i] == 1 ? next_row : next_col, 2);

				// Second/last side.
				if (real = chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && (concurrent_piece.type == chess::piece::EMPTY || (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory))) {
					pos_list.push_back(pos);
				}

				// Reset/go next.
				real = false;
			}

			// Reset stage and concurrent stuff.
			if (concurrent_i == 2 || !real) {
				previous_i = i;
				concurrent_stage = 0;

				// Break at three stage.
				if (concurrent_mask_i == 3) {
					break;
				}

				concurrent_mask_i++;

				next_row = row;
				next_col = col;
			}
		}
		/* End of horse path finder. */
	} else if (type == chess::piece::BISHOP) {
		/* Start of bishop path finder. */
		// Concurrent iterations from difference and stages.
		uint8_t concurrent_mask_i = 0;
		uint8_t concurrent_i = 0;
		uint8_t previous_i = 0;

		// Set new values for directions:
		chess::matrix::unalign(mask_factor_flag_1, mask_factor_flag_2);

		// 1..33 = 32
		for (uint8_t i = 1; i < 33; i++) {
			concurrent_i = i - previous_i;

			// Move with base in stage (direction)
			chess::matrix::move(next_row, mask_factor_flag_1[concurrent_mask_i]);
			chess::matrix::move(next_col, mask_factor_flag_2[concurrent_mask_i]);

			// Verify if contains an empty slot or if is an enemy.
			real = chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && (concurrent_piece.type == chess::piece::EMPTY || (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory));	
			
			if (real) {
				pos_list.push_back(pos);

				// If is an enemy stop this stage and go next.
				if (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory) {
					real = false;
				}
			}

			// For move the stages.
			if (concurrent_i == 8 || !real) {
				previous_i = i;

				// Break at four stage.
				if (concurrent_mask_i == 3) {
					break;
				}

				concurrent_mask_i++;

				next_row = row;
				next_col = col;
			}
		}
		/* End of bishop path finder. */	
	} else if (type == chess::piece::QUEEN) { // ME <3 Rina
		/* Start of rina path finder. */
		bool next_stage = false;
		bool next_stage_fix_matrix = false;

		// Concurrent iterations from difference and stages.
		uint8_t concurrent_mask_i = 0;
		uint8_t concurrent_i = 0;
		uint8_t previous_i = 0;

		// Set unligned matrix direction.
		chess::matrix::unalign(mask_factor_flag_1, mask_factor_flag_2);

		// 1..65 = 64
		for (uint8_t i = 1; i < 65; i++) {
			concurrent_i = i - previous_i;

			// Move with base in stage (direction)
			if (next_stage) {
				if (next_stage_fix_matrix) {
					chess::matrix::align(mask_factor_flag_1, mask_factor_flag_2);
					next_stage_fix_matrix = false;
				}

				// Move with base in stage (direction)
				chess::matrix::move(mask_factor_flag_2[concurrent_mask_i] == 1 ? next_col : next_row, mask_factor_flag_1[concurrent_mask_i]);
			} else {
				chess::matrix::move(next_row, mask_factor_flag_1[concurrent_mask_i]); // The row force.
				chess::matrix::move(next_col, mask_factor_flag_2[concurrent_mask_i]); // The col force.
			}

			// Verify if contains an empty slot or if is an enemy.
			real = chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && (concurrent_piece.type == chess::piece::EMPTY || (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory));	
			
			if (real) {
				pos_list.push_back(pos);

				// If is an enemy stop this stage and go next.
				if (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory) {
					real = false;
				}
			}

			// For move the stages.
			if (concurrent_i == 8 || !real) {
				previous_i = i;
				real = false;

				// Break at four stage.
				if (concurrent_mask_i == 3) {
					if (!next_stage) {
						next_stage = true;
						next_stage_fix_matrix = true;
						concurrent_mask_i = 0;
						real = true;
					} else {
						break;
					}
				}

				if (!real) {
					concurrent_mask_i++;
				}

				next_row = row;
				next_col = col;
			}
		}
		/* End of queen path finder. */
	} else if (type == chess::piece::KING) {
		/* Start of king path finder. */
		bool next_stage = false;
		bool next_stage_fix_matrix = false;

		// Concurrent iterations from difference and stages.
		uint8_t concurrent_mask_i = 0;
		uint8_t concurrent_i = 0;
		uint8_t previous_i = 0;

		// Set unligned matrix direction.
		chess::matrix::unalign(mask_factor_flag_1, mask_factor_flag_2);

		// 1..65 = 64
		for (uint8_t i = 1; i < 9; i++) {
			concurrent_i = i - previous_i;

			// Move with base in stage (direction)
			if (next_stage) {
				if (next_stage_fix_matrix) {
					chess::matrix::align(mask_factor_flag_1, mask_factor_flag_2);
					next_stage_fix_matrix = false;
				}

				// Move with base in stage (direction)
				chess::matrix::move(mask_factor_flag_2[concurrent_mask_i] == 1 ? next_col : next_row, mask_factor_flag_1[concurrent_mask_i]);
			} else {
				chess::matrix::move(next_row, mask_factor_flag_1[concurrent_mask_i]);
				chess::matrix::move(next_col, mask_factor_flag_2[concurrent_mask_i]);
			}

			// Verify if contains an empty slot or if is an enemy.
			real = chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && (concurrent_piece.type == chess::piece::EMPTY || (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory));	
			
			if (real) {
				pos_list.push_back(pos);

				// If is an enemy stop this stage and go next.
				if (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory) {
					real = false;
				}
			}

			// For move the stages.
			if (concurrent_i == 1 || !real) {
				previous_i = i;
				real = false;

				// Break at four stage.
				if (concurrent_mask_i == 3) {
					if (!next_stage) {
						next_stage = true;
						next_stage_fix_matrix = true;
						concurrent_mask_i = 0;
						real = true;
					} else {
						break;
					}
				}

				if (!real) {
					concurrent_mask_i++;
				}

				next_row = row;
				next_col = col;
			}
		}
		/* End of king path finder. */
	}
}

void chess::matrix::vec(uint8_t _vec[2], uint8_t pos) {
	uint8_t last_jump = 0;
	uint8_t iterator = 0;

	uint8_t concurrent_jump = 0;
	uint8_t concurrent_rows = 0;
	uint8_t concurrent_cols = 0;

	// + 1 (the inital index is 0 so... 0..63 = 64; but 1..63 = 63; so 1..64 = 64);
	for (uint8_t jumps = 1; jumps < 64 + 1; jumps++) {
		if (iterator == pos) {
			_vec[0] = concurrent_rows;
			_vec[1] = concurrent_cols;

			break;
		}

		iterator++;
		concurrent_jump = jumps - last_jump;

		if (concurrent_jump == 8) {
			last_jump = jumps;

			if (concurrent_cols != 8) {
				concurrent_cols++;
			}

			concurrent_rows = 0;
		} else {
			concurrent_rows++;
		}
	}
}

uint8_t chess::matrix::find(uint8_t row, uint8_t col) {
	uint8_t last_jump = 0;
	uint8_t iterator = 0;

	uint8_t concurrent_jump = 0;
	uint8_t concurrent_rows = 0;
	uint8_t concurrent_cols = 0;

	// + 1 (the inital index is 0 so... 0..63 = 64; but 1..63 = 63; so 1..64 = 64);
	for (uint8_t jumps = 1; jumps < 64 + 1; jumps++) {
		if (concurrent_rows == row && concurrent_cols == col) {
			return iterator;
		}

		iterator++;
		concurrent_jump = jumps - last_jump;

		if (concurrent_jump == 8) {
			last_jump = jumps;

			if (concurrent_cols != 8) {
				concurrent_cols++;
			}

			concurrent_rows = 0;
		} else {
			concurrent_rows++;
		}
	}

	return chess::OUT_RANGE;
}

bool chess::get(entity_piece &pass_entity, uint8_t pos) {
	for (entity_piece &entity : chess::loaded_entity_list) {
		if (entity.pos == pos) {
			pass_entity = entity;
			return true;
		}
	}

	return false;
}

bool chess::relative_height(entity_piece &entity, uint8_t color_factory, float &height, uint8_t i) {
	float full = (chess::square_size * i);

	if (color_factory == 0) {
		height = chess::white_dock == chess::TOP ? full : -entity.h;
	} else {
		height = chess::white_dock == chess::BOTTOM ? full : -entity.h;
	}

	return height != full;
}

bool chess::entities_bouding_box_collide(entity_piece &entity_1, entity_piece &entity_2) {
	float x = entity_1.x;
	float y = entity_1.y;
	float w = x + entity_1.w;
	float h = y + entity_1.h;

	return x < entity_2.x + entity_2.w && w > entity_2.x &&
		   y < entity_2.y + entity_2.h && h > entity_2.y;
}

void chess::crawl_to_the_ressurection(entity_piece &the_entity) {
	the_entity.the_god_is_trying_to_talk_with_you = false;

	if (the_entity.piece_slot.type != chess::piece::PAWN) {
		return;
	}

	uint8_t flag = 0;
	uint8_t vec[2];

	chess::matrix::vec(vec, the_entity.pos);

	// 1 white; 0 black.
	if (the_entity.color_factory) {
		flag = chess::white_dock == chess::TOP ? vec[1] == 7 : vec[1] == 0;
	} else {
		flag = chess::white_dock == chess::BOTTOM ? vec[1] == 7 : vec[1] == 0;
	}

	the_entity.the_god_is_trying_to_talk_with_you = flag;
}

void chess::creep_4_tha_death(entity_piece &the_death_as_an_entity_piece, uint8_t &entity_type_killed) {
	// Verify if an entity pos is equals to the moved piece 'the_death_as_an_entity_piece'.
	// Always based on color to prevent moved entity self-collide.
	for (entity_piece &entity : chess::loaded_entity_list) {
		if (!entity.dead && entity.color_factory != the_death_as_an_entity_piece.color_factory && entity.pos == the_death_as_an_entity_piece.pos && chess::entities_bouding_box_collide(the_death_as_an_entity_piece, entity)) {
			entity.kill(the_death_as_an_entity_piece.color_factory);
			entity_type_killed = entity.piece_slot.type;

			// Set to air the concurrent entity.pos.
			chess::map[entity.pos].type = chess::piece::EMPTY;
			break;
		}
	}

	// The name is just a joke, gangstar yooo drip.
}

void chess::set_piece(entity_piece &entity, uint8_t type) {
	entity.set(chess::piece_type_map[type]);
}

void chess::move(entity_piece &entity, uint8_t pos) {
	// Set to "air" the old slot.
	chess::map[entity.pos].type = chess::piece::EMPTY;

	// Set the new slot.
	chess::map[pos].type = entity.piece_slot.type;
	entity.pos = pos;

	uint8_t vec[2];
	chess::matrix::vec(vec, pos);

	// Set entity position.
	entity.x = chess::map[0].x + (vec[0] * chess::square_size);
	entity.y = chess::map[0].y + (vec[1] * chess::square_size);
}

void chess::init(SDL_Window* sdl_window) {
	this->refresh(sdl_window);

	if (!util::file::read_texture(texture, GL_RGBA, "data/textures/chess-symbols.png")) {
		util::log("Could not load chess symbols texture.");
	}

	util::log("Chess game core initialized.");
}

void chess::new_game() {
	this->gaming = true;
	this->previous_color_moved = 0;
	this->refresh();

	chess::white_dock = chess::white_dock == chess::TOP ? chess::BOTTOM : chess::TOP;
	chess::loaded_entity_list.clear();

	uint8_t not_infantry[8] = {piece::TOWER, piece::HORSE, piece::BISHOP, piece::KING, piece::QUEEN, piece::BISHOP, piece::HORSE, piece::TOWER};
	uint8_t ordened_count = 0;

	bool pawn_sector = true;

	for (uint8_t i = 0; i < 16; i++) {
		entity_piece piece_white, piece_black;

		piece_white.color_factory = 1;
		piece_black.color_factory = 0;

		piece_white.piece_slot.color = 1;
		piece_black.piece_slot.color = 0;

		piece_white.w = chess::square_size;
		piece_white.h = chess::square_size;

		piece_black.w = chess::square_size;
		piece_black.h = chess::square_size;

		if (pawn_sector) {
			piece_white.set(chess::render::pawn);
			piece_black.set(chess::render::pawn);

			piece_white.x = this->x + (chess::square_size * ordened_count);
			piece_black.x = this->x + (chess::square_size * ordened_count);

			// Set entity color.
			chess::render::set_color(piece_white, chess::color::WHITE);
			chess::render::set_color(piece_black, chess::color::BLACK);

			if (chess::white_dock == chess::TOP) {
				piece_white.y = this->y + chess::square_size;
				piece_black.y = this->y + (chess::square_size) * 6;

				// Set in map.
				uint8_t pos = chess::matrix::find(ordened_count, 1);
				chess::move(piece_white, pos);

				pos = chess::matrix::find(ordened_count, 6);
				chess::move(piece_black, pos);
			} else {
				piece_white.y = this->y + (chess::square_size) * 6;
				piece_black.y = this->y + chess::square_size;

				// Set in map.
				uint8_t pos = chess::matrix::find(ordened_count, 6);
				chess::move(piece_white, pos);

				pos = chess::matrix::find(ordened_count, 1);
				chess::move(piece_black, pos);
			}

			if (ordened_count == 7) {
				pawn_sector = false;
				ordened_count = 0;
			} else {
				ordened_count++;
			}
		} else {
			uint8_t o = not_infantry[ordened_count];

			chess::set_piece(piece_white, o);
			chess::set_piece(piece_black, o);

			piece_white.x = this->x + (chess::square_size * ordened_count);
			piece_black.x = this->x + (chess::square_size * ordened_count);

			// Set entity color.
			chess::render::set_color(piece_white, chess::color::WHITE);
			chess::render::set_color(piece_black, chess::color::BLACK);

			if (chess::white_dock == chess::TOP) {
				piece_white.y = this->y;
				piece_black.y = this->y + (chess::square_size) * 7;

				// Set in map.
				uint8_t pos = chess::matrix::find(ordened_count, 0);
				chess::move(piece_white, pos);

				pos = chess::matrix::find(ordened_count, 7);
				chess::move(piece_black, pos);
			} else {
				piece_white.y = this->y + (chess::square_size) * 7;
				piece_black.y = this->y;

				// Set in map.
				uint8_t pos = chess::matrix::find(ordened_count, 7);
				chess::move(piece_white, pos);

				pos = chess::matrix::find(ordened_count, 0);
				chess::move(piece_black, pos);
			}

			ordened_count++;
		}

		// Set direct pos.
		piece_white.previous_x = piece_white.x;
		piece_white.previous_y = piece_white.y;

		piece_black.previous_x = piece_black.x;
		piece_black.previous_y = piece_black.y;

		// Add at loaded render list.
		chess::loaded_entity_list.push_back(piece_white);
		chess::loaded_entity_list.push_back(piece_black);
	}
}

void chess::end_game() {
	this->gaming = false;
}

void chess::set_pos(float pos_x, float pos_y) {
	if (this->x != pos_x || this->y != pos_y) {
		float old_pos_x = 0, old_pos_y = 0;

		for (entity_piece &entity : chess::loaded_entity_list) {
			old_pos_x = entity.x - this->x;
			old_pos_y = entity.y - this->y;

			entity.x = pos_x + old_pos_x;
			entity.y = pos_y + old_pos_y;

			entity.previous_x = entity.x;
			entity.previous_y = entity.y;
		}

		this->x = pos_x;
		this->y = pos_y;
	}
}

void chess::refresh(SDL_Window* sdl_window) {
	if (sdl_window != nullptr) {
		int32_t w, h;
		SDL_GetWindowSize(sdl_window, &w, &h);

		this->screen_w = (float) w;
		this->screen_h = (float) h;
	}

	this->w = 8 * chess::square_size;
	this->h = 8 * chess::square_size;

	// Iterator for map stuff.
	uint8_t iterator = 0;

	// The for loop variables does not affect nothing.
	uint8_t last_jump = 0;

	uint8_t concurrent_jump = 0;
	uint8_t concurrent_rows = 0;
	uint8_t concurrent_cols = 0;

	// + 1 (the inital index is 0 so... 0..63 = 64; but 1..63 = 63; so 1..64 = 64);
	for (uint8_t jumps = 1; jumps < 64 + 1; jumps++) {
		piece_data pd;

		pd.x = this->x + (concurrent_rows * chess::square_size);
		pd.y = this->y + (concurrent_cols * chess::square_size);

		pd.w = square_size;
		pd.h = square_size;
		pd.type = chess::piece::EMPTY;
 
		chess::map[iterator++] = pd;
		concurrent_jump = jumps - last_jump;

		if (concurrent_jump == 8) {
			last_jump = jumps;

			if (concurrent_cols != 8) {
				concurrent_cols++;
			}

			concurrent_rows = 0;
		} else {
			concurrent_rows++;
		}
	}
}

bool chess::contains(uint8_t pos) {
	for (uint8_t positions : this->possible) {
		if (positions == pos) {
			return true;
		}
	}

	return false;
}

void chess::on_event(SDL_Event &sdl_event) {
	switch (sdl_event.type) {
		case SDL_MOUSEBUTTONDOWN: {
			float x = sdl_event.motion.x;
			float y = sdl_event.motion.y;

			this->over = (x > this->x && y > this->y && x < this->x + this->w && y < this->y + this->h);

			if (!this->over && !this->ressurection) {
				this->possible.clear();
				this->start_pos = false;
				this->end_pos = false;

				break;
			}

			if (this->ressurection && x > this->rina_notify.x && y > this->rina_notify.y && x < this->rina_notify.x + this->rina_notify.w && y < this->rina_notify.y + this->rina_notify.h) {
				for (entity_piece &entity : chess::loaded_entity_list) {
					if (entity.pos == this->focused) {
						entity.set(chess::render::queen);
						entity.set_color(this->rina_notify.color_factory);
						chess::render::set_color(entity, chess::render::get_color(entity));

						chess::map[entity.pos].type = chess::piece::QUEEN;
						break;
					}
				}

				this->ressurection = false;
				break;
			}

			bool start_flag;

			uint8_t pos;
			uint8_t new_pos;

			if (this->start_pos) {
				for (piece_data &pieces : chess::map) {
					// If mouse is over.
					if (x > pieces.x && y > pieces.y && x < pieces.x + pieces.w && y < pieces.y + pieces.h) {
						// Find vec pos and kiss me.
						chess::matrix::from(pieces.x - this->x, pieces.y - this->y, this->matrix_pos[0], this->matrix_pos[1]);
						pos = chess::matrix::find(this->matrix_pos[0], this->matrix_pos[1]);

						// If not contains in possible places we pass this tick.
						if (!this->contains(pos)) {
							continue;
						}

						// Set the next/new pos.
						new_pos = pos;

						// Find for the old matrix pos.
						chess::matrix::from(this->start.x - this->x, this->start.y - this->y, this->matrix_pos[0], this->matrix_pos[1]);
						pos = chess::matrix::find(this->matrix_pos[0], this->matrix_pos[1]);

						// Set the current/old pos.
						this->matrix_pos[0] = pos;
						this->matrix_pos[1] = new_pos;

						// Now go to for above (to down).
						start_flag = true;
						break;
					}
				}
			}

			uint8_t hovered = chess::OUT_RANGE;
			uint8_t entity_type_killed = 0;

			entity_piece entity;

			for (entity_piece &entity : chess::loaded_entity_list) {
				if (entity.dead) {
					if (this->ressurection && x > entity.x && y > entity.y && x < entity.x + entity.w && y < entity.y + entity.h && entity.color_factory == this->color_ressure) {
						for (entity_piece &e : chess::loaded_entity_list) {
							if (e.pos == this->focused) {
								e.kill(chess::KILLED_TO_ETERNETIY);
								break;
							}
						}

						entity.w = chess::square_size;
						entity.h = chess::square_size;
						entity.ressure(this->focused);

						uint8_t vec[2];
						chess::matrix::vec(vec, this->focused);

						entity.x = this->x + (vec[0] * chess::square_size);
						entity.y = this->y + (vec[1] * chess::square_size);

						this->ressurection = false;
						break;
					}

					continue;
				}

				if (start_flag) {
					// 0 == current/old position; 1 == next/new position.
					if (entity.pos == this->matrix_pos[0]) {
						// Move and set moved flag.
						chess::move(entity, this->matrix_pos[1]);
						entity.piece_slot.moved = true;

						// Killa section...
						if (!this->gamemode_godmode) {
							chess::creep_4_tha_death(entity, entity_type_killed);
							chess::crawl_to_the_ressurection(entity);
						}

						if (entity.the_god_is_trying_to_talk_with_you) {
							this->focused = entity.pos;
							this->color_ressure = entity.color_factory;
							this->ressurection = true;
						}

						if (entity_type_killed == chess::piece::KING) {
							util::log(std::string("YO! ") + std::string((entity.color_factory == 0 ? "white" : "black")) + " side wins!");
							this->end_game();
						}

						this->start_pos = false;
						this->end_pos = false;
						this->possible.clear();

						// 0 black; 1 white.
						this->previous_color_moved = this->concurrent_color_moved;
						break;
					}

					continue;
				}

				if (x > entity.x && y > entity.y && x < entity.x + entity.w && y < entity.y + entity.h && !this->ressurection && (!this->gamemode_cycle || this->gamemode_cycle && this->previous_color_moved != entity.color_factory)) {
					this->concurrent_color_moved = entity.color_factory;
					this->possible.clear();

					this->start.x = entity.x;
					this->start.y = entity.y;
					this->start.w = entity.w;
					this->start.h = entity.h;

					this->start_pos = true;
					this->end_pos = false;

					chess::matrix::from(entity.x - this->x, entity.y - this->y, this->matrix_pos[0], this->matrix_pos[1]);
					chess::matrix::possible(this->possible, entity.piece_slot.type, entity.color_factory, this->matrix_pos[0], this->matrix_pos[1]);

					hovered = entity.pos;
					break;
				}
			}

			if (hovered == chess::OUT_RANGE) {
				this->possible.clear();
				this->start_pos = false;
				this->end_pos = false;
			}

			break;
		}

		case SDL_MOUSEMOTION: {
			float x = sdl_event.motion.x;
			float y = sdl_event.motion.y;

			this->over = (x > this->x && y > this->y && x < this->x + this->w && y < this->y + this->h);
			this->my = y;

			if (this->over) {
				for (piece_data &pieces : chess::map) {
					if (x > pieces.x && y > pieces.y && x < pieces.x + pieces.w && y < pieces.y + pieces.h) {
						this->hovered = pieces;
						break;
					}
				}
			}

			break;
		}

		case SDL_MOUSEBUTTONUP: {
			break;
		}
	}
}

void chess::on_update(uint64_t delta) {
	this->update = delta > 800;

	if (delta > 500 && delta < 600 && !this->update) {
		float height = .0f;

		// There is two axis to handle.
		float axis_white = 0;
		float axis_black = 0;

		float axis_diff = chess::square_size / 2;
		float axis_sync = 0;

		// Set position when kill some shit.
		for (entity_piece &entity : chess::loaded_entity_list) {
			if (!entity.dead || entity.color_of_tha_death == chess::KILLED_TO_ETERNETIY) {
				continue;
			}

			// Get relative height of color.
			entity.w = axis_diff;
			entity.h = axis_diff;
			chess::relative_height(entity, entity.color_of_tha_death, height);

			axis_sync = (entity.color_factory ? axis_white : axis_black);

			// Set position based on relative height.
			entity.x = this->x + axis_sync;
			entity.y = this->y + height;

			// Update axis.
			axis_white += entity.color_factory ? entity.w : 0;
			axis_black += entity.color_factory ? 0 : entity.w;
		}

		this->update = true;
	}
}

void chess::on_render(float render_ticks) {
	if (!this->gaming) {
		return;
	}

	tessellator::fx(fx_manager::light_specular_fx);

	// Set the specular light position.
	fx_manager::light_specular_fx.use();
	fx_manager::light_specular_fx.set_float("x", this->x + (this->w / 2));
	fx_manager::light_specular_fx.set_float("y", this->y + (this->h / 2));
	fx_manager::light_specular_fx.set_float("scale", 2.0f);

	util::render::shape(0, 0, this->screen_w, this->screen_h, util::color(255, 255, 255, 200));
	tessellator::fx();

	float width = chess::square_size / 4;
	float offset = width + (width / 2);

	if (this->gamemode_cycle) {
		float color_fill_factor = this->previous_color_moved ? 0 : 255;

		// Draw the concurrent color to move.
		tessellator::fx(fx_manager::light_specular_fx);

		// Set the specular light position.
		fx_manager::light_specular_fx.use();
		fx_manager::light_specular_fx.set_float("x", this->x + offset);
		fx_manager::light_specular_fx.set_float("y", util::math::clampf(this->my, this->y, this->y + this->h));
		fx_manager::light_specular_fx.set_float("scale", 1.0f);

		util::render::shape(this->x - offset, this->y, width, this->h, util::color(color_fill_factor, color_fill_factor, color_fill_factor, this->alpha));
		tessellator::fx();
	}

	if (this->ressurection) {
		float height = 0;

		this->rina_notify.w = chess::square_size;
		this->rina_notify.h = chess::square_size;

		if (chess::relative_height(this->rina_notify, this->color_ressure == 0 ? 1 : 0, height, 7)) {
			height += rina_notify.h;
		}

		this->rina_notify.x = this->x - width - offset - chess::square_size;
		this->rina_notify.y = this->y + height;
		this->rina_notify.set(chess::render::queen);
		this->rina_notify.set_color(this->color_ressure);

		chess::render::set_color(this->rina_notify, chess::render::get_color(this->rina_notify));
		this->rina_notify.on_render(render_ticks);
	}

	this->color_white.a = this->alpha;
	this->color_black.a = this->alpha;

	color tile_color = color::WHITE;
	uint8_t t = 0;

	for (piece_data &places : chess::map) {
		// Swap color.
		tile_color = tile_color == color::WHITE ? color::BLACK : color::WHITE;

		if (t == 8) {
			tile_color = tile_color == color::WHITE ? color::BLACK : color::WHITE;
			t = 0;
		}

		// Render the tiles.
		util::render::shape(places.x, places.y, places.w, places.h, tile_color == color::WHITE ? this->color_white : this->color_black);
		t++;
	}

	if (this->start_pos) {
		/* Start of rendering start pos. */
		fx_manager::light_specular_fx.use();
		fx_manager::light_specular_fx.set_float("x", this->start.x + (this->start.w / 2));
		fx_manager::light_specular_fx.set_float("y", this->start.y + (this->start.h / 2));

		tessellator::fx(fx_manager::light_specular_fx);
		util::render::shape(this->start.x, this->start.y, this->start.w, this->start.h, util::color(0, 0, 255, 100));
		tessellator::fx();
		/* End of rendering start pos. */

		piece_data places;

		// Draw each possible place from to piece selected.
		for (uint8_t index_pos : this->possible) {
			places = chess::map[index_pos];

			/* Start of rendering places. */
			fx_manager::light_specular_fx.use();
			fx_manager::light_specular_fx.set_float("x", places.x + (places.w / 2));
			fx_manager::light_specular_fx.set_float("y", places.y + (places.h / 2));

			tessellator::fx(fx_manager::light_specular_fx);
			util::render::shape(places.x, places.y, places.w, places.h, util::color(255, 0, 0, 50));
			tessellator::fx();
			/* End of rendering places. */
		}
	}

	if (this->over) {
		util::render::shape_outline(this->hovered.x, this->hovered.y, this->hovered.w, this->hovered.h, 1.0f, util::color(0, 255, 0, 50));
	}

	for (entity_piece &entity : chess::loaded_entity_list) {
		if (entity.color_of_tha_death != chess::KILLED_TO_ETERNETIY) {
			entity.on_render(render_ticks);
		}
	}
}