#include "chess.h"
#include "tessellator.h"
#include "shader.h"

util::texture chess::texture = util::texture();
uint8_t chess::white_dock = chess::TOP;
float chess::square_size = 30;

piece_data chess::render::queen  = piece_data();
piece_data chess::render::bishop = piece_data();
piece_data chess::render::pawn   = piece_data();
piece_data chess::render::tower  = piece_data();
piece_data chess::render::horse  = piece_data();
piece_data chess::render::king   = piece_data();

uint8_t chess::OUT_RANGE = 66;
uint8_t chess::TOP       = 67;
uint8_t chess::DOWN      = 68;

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

void entity_piece::set(piece_data &_piece_data) {
	this->piece_slot = _piece_data;
	this->alive = true;
}

void entity_piece::on_render(float render_ticks) {
	this->previous_x = util::math::lerpf(this->previous_x, this->x, render_ticks);
	this->previous_y = util::math::lerpf(this->previous_y, this->y, render_ticks);

	// Draw the piece.
	util::render::shape_texture(this->previous_x, this->previous_y, this->w, this->h, this->piece_slot.x, this->piece_slot.y, this->piece_slot.w, this->piece_slot.h, this->texture);
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

void chess::matrix::move(uint8_t &not_matrix_pos, uint8_t velocity) {
	not_matrix_pos += velocity;
	not_matrix_pos = util::math::clampi(not_matrix_pos, 0, 8);
}

bool chess::matrix::get(piece_data &_piece_data, uint8_t pos) {
	if (pos > 64) {
		return false;
	}

	_piece_data = chess::map[pos];

	for (entity_piece &entity : chess::loaded_entity_list) {
		if (entity.pos == pos) {
			_piece_data = entity.piece_slot;
			_piece_data.color = entity.color_factory;
			break;
		}
	}

	return true;
}

void chess::matrix::possible(std::vector<uint8_t> &pos_list, uint8_t type, uint8_t color_factory, uint8_t row, uint8_t col) {
	bool white = ((bool) color_factory);
	bool real = false;

	piece_data concurrent_piece;

	uint8_t pos = 0;
	uint8_t next_row = row;
	uint8_t next_col = col;

	// Directions.
	int8_t directions[2] = {1, -1};

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

		uint8_t concurrent_stage = 0;
		uint8_t concurrent_i = 0;
		uint8_t previous_i = 0;

		// 1..33 = 32
		for (uint8_t i= 1; i < 65; i++) {
			concurrent_i = i - previous_i;

			if (concurrent_stage == 0) {
				chess::matrix::move(next_col, 1);
			} else if (concurrent_stage == 1) {
				chess::matrix::move(next_row, 1);
			} else if (concurrent_stage == 2) {
				chess::matrix::move(next_col, -1);
			} else if (concurrent_stage == 3) {
				chess::matrix::move(next_row, -1);
			}

			real = chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && (concurrent_piece.type == chess::piece::EMPTY || (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory));	
			
			if (real) {
				pos_list.push_back(pos);

				if (concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color != color_factory) {
					real = false;
				}
			}

			if (concurrent_i == 8 || !real) {
				concurrent_i = i;
				concurrent_stage++;

				if (concurrent_stage > 4) {
					break;
				}

				next_row = row;
				next_col = col;
			}
		}
		/* End of tower path finder. */
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


void chess::set_piece(entity_piece &entity, uint8_t type) {
	entity.set(chess::piece_type_map[type]);
}

void chess::move(entity_piece &entity, uint8_t pos) {
	piece_data _piece_data;

	// Set to "air" the old slot.
	_piece_data = chess::map[entity.pos];
	_piece_data.type = chess::piece::EMPTY;
	chess::map[entity.pos] = _piece_data;

	// Set the new slot.
	_piece_data = chess::map[pos];
	_piece_data.type = entity.piece_slot.type;
	chess::map[pos] = _piece_data;		
	entity.pos = pos;

	// Set entity position.
	uint8_t vec[2];
	chess::matrix::vec(vec, pos);

	entity.x = chess::map[0].x + (vec[0] * chess::square_size);
	entity.y = chess::map[0].y + (vec[1] * chess::square_size);
}

void chess::init() {
	this->refresh();

	if (!util::file::read_texture(texture, GL_RGBA, "data/textures/chess-symbols.png")) {
		util::log("Could not load chess symbols texture.");
	}

	util::log("Chess game core initialized.");
}

void chess::new_game() {
	this->gaming = true;
	chess::loaded_entity_list.clear();
	uint8_t not_infantry[8] = {piece::TOWER, piece::HORSE, piece::BISHOP, piece::KING, piece::QUEEN, piece::BISHOP, piece::HORSE, piece::TOWER};

	bool pawn_sector = true;
	uint8_t ordened_count = 0;

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

			piece_white.texture = chess::texture;
			piece_black.texture = chess::texture;

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

			piece_white.texture = chess::texture;
			piece_black.texture = chess::texture;

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
			
		// Add at loaded render list.
		chess::loaded_entity_list.push_back(piece_white);
		chess::loaded_entity_list.push_back(piece_black);
	}
}

void chess::end_game() {
	this->gaming = false;
}

void chess::refresh() {
	SDL_DisplayMode sdl_display_mode;
	SDL_GetDisplayMode(0, 0, &sdl_display_mode);

	this->screen_w = sdl_display_mode.w;
	this->screen_h = sdl_display_mode.h;

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

			if (!this->over) {
				this->possible.clear();
				this->start_pos = false;
				this->end_pos = false;

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

			this->focused = chess::OUT_RANGE;
			entity_piece entity;

			for (entity_piece &entity : chess::loaded_entity_list) {
				if (start_flag) {
					// 0 == current/old position; 1 == next/new position.
					if (entity.pos == this->matrix_pos[0]) {
						// Move and flag that was moved.
						chess::move(entity, this->matrix_pos[1]);
						entity.piece_slot.moved = true;

						this->start_pos = false;
						this->end_pos = false;
						this->possible.clear();
							
						break;
					}

					continue;
				}

				if (x > entity.x && y > entity.y && x < entity.x + entity.w && y < entity.y + entity.h) {
					this->possible.clear();

					this->start.x = entity.x;
					this->start.y = entity.y;
					this->start.w = entity.w;
					this->start.h = entity.h;

					this->start_pos = true;
					this->end_pos = false;

					chess::matrix::from(entity.x - this->x, entity.y - this->y, this->matrix_pos[0], this->matrix_pos[1]);
					chess::matrix::possible(this->possible, entity.piece_slot.type, entity.color_factory, this->matrix_pos[0], this->matrix_pos[1]);
						
					this->focused = entity.pos;
					break;
				}
			}

			if (this->focused == chess::OUT_RANGE) {
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

void chess::on_render(float render_ticks) {
	if (!this->gaming) {
		return;
	}

	tessellator::fx(fx_manager::light_specular_fx);

	// Set the specular light position.
	fx_manager::light_specular_fx.use();
	fx_manager::light_specular_fx.set_float("x", this->x + (this->w / 2));
	fx_manager::light_specular_fx.set_float("y", this->y + (this->h / 2));
	fx_manager::light_specular_fx.set_float("scale", 1.2f);

	util::render::shape(0, 0, this->screen_w, this->screen_h, util::color(255, 255, 255, 100));
	tessellator::fx();

	this->color_white.a = alpha;
	this->color_black.a = alpha;

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
		piece_data places;

		// Draw each possible place from to piece selected.
		for (uint8_t index_pos : this->possible) {
			places = chess::map[index_pos];

			// Render the end pos.
			util::render::shape(places.x, places.y, places.w, places.h, util::color(255, 0, 0, 50));
		}
	}

	for (entity_piece &entity : chess::loaded_entity_list) {
		entity.on_render(render_ticks);
	}

	if (this->over) {
		util::render::shape_outline(this->hovered.x, this->hovered.y, this->hovered.w, this->hovered.h, 1.0f, util::color(0, 255, 0, 50));
	}

	if (this->dragging) {
		util::render::shape(this->hovered.x, this->hovered.y, this->hovered.w, this->hovered.h, util::color(0, 0, 255, 50));
	}
}