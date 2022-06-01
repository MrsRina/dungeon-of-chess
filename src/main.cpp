#include "includes.h"
#include "util.h"
#include "tessellator.h"

uint32_t screen_w = 800;
uint32_t screen_h = 600;

struct piece_data {
	float x, y, w, h;

	uint8_t type;
	uint8_t color;

	void set(float rect_x, float rect_y, float rect_w, float rect_h) {
		this->x = rect_x;
		this->y = rect_y;
		this->w = rect_w;
		this->h = rect_h;
	}
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

	void set_color(uint8_t color) {
		this->color_factory = color;
		this->piece_slot.color = color;
	}

	void set(piece_data &_piece_data) {
		this->piece_slot = _piece_data;
		this->alive = true;
	}

	void on_render(float render_ticks) {
		// Draw the piece.
		util::render::shape_texture(this->x, this->y, this->w, this->h, this->piece_slot.x, this->piece_slot.y, this->piece_slot.w, this->piece_slot.h, this->texture);
	}
};

struct chess {
	const static uint8_t OUT_RANGE = 66;
	const static uint8_t TOP       = 67;
	const static uint8_t DOWN      = 68;

	struct piece {
		const static uint8_t PAWN   = 0;
		const static uint8_t TOWER  = 1;
		const static uint8_t HORSE  = 2;
		const static uint8_t BISHOP = 3;
		const static uint8_t QUEEN  = 4;
		const static uint8_t KING   = 5;
		const static uint8_t EMPTY  = 6;
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

		static void init() {
			float width = chess::texture.width / 6;
			float height = chess::texture.height / 2;

			queen.set(0.0f, 0.0F, width, height);
			queen.type = chess::piece::QUEEN;

			king.set(width * 1, 0.0F, width, height);
			king.type = chess::piece::KING;

			tower.set(width * 2, 0.0F, width, height);
			tower.type = chess::piece::TOWER;

			horse.set(width * 3, 0.0F, width, height);
			horse.type = chess::piece::HORSE;

			bishop.set(width * 4, 0.0F, width, height);
			bishop.type = chess::piece::BISHOP;

			pawn.set(width * 5, 0.0F, width, height);
			pawn.type = chess::piece::PAWN;
		}

		static chess::color get_color(entity_piece &entity) {
			return entity.color_factory == 1 ? chess::color::WHITE : chess::color::BLACK;
		}

		static void set_color(entity_piece &entity, chess::color piece_color) {
			float height = chess::texture.height / 2;
			entity.piece_slot.y = piece_color == chess::color::WHITE ? height : 0.0f;
		}
	};

	struct matrix {
		static void from(float x, float y, uint8_t &row, uint8_t &col) {
			row = (uint8_t) (x / chess::square_size);
			col = (uint8_t) (y / chess::square_size);

			row = util::math::clampi(row, 0, 8); 
			col = util::math::clampi(col, 0, 8);
		}

		static void move(uint8_t &not_matrix_pos, uint8_t velocity) {
			not_matrix_pos += velocity;
			not_matrix_pos = util::math::clampi(not_matrix_pos, 0, 8);
		}

		static bool get(piece_data &_piece_data, uint8_t pos) {
			if (pos > 64) {
				return false;
			}

			_piece_data = chess::map[pos];

			for (entity_piece &entity : chess::loaded_entity_list) {
				if (entity.pos == pos) {
					util::log("Found ()p " + std::to_string(pos));
					_piece_data = entity.piece_slot;
					break;
				}
			}

			return true;
		}

		static void possible(std::vector<uint8_t> &pos_list, uint8_t type, uint8_t color_factory, uint8_t row, uint8_t col) {
			bool white = ((bool) color_factory);
			bool real = false;

			piece_data concurrent_piece;

			uint8_t pos = 0;
			uint8_t next_row = row;
			uint8_t next_col = col;

			// Velocity based on side positions.
			int8_t v_white = chess::white_dock == chess::TOP ?  1 : -1;
			int8_t v_black = chess::white_dock == chess::TOP ? -1 :  1;

			// We have a loop with 256 ticks.
			// 64 * 4 = 256
			for (uint8_t ticks = 0; ticks < 256; ticks++) {
				if (type == chess::piece::PAWN) {
					// Move the piece to front (based in velocity).
					chess::matrix::move(next_col, white ? v_white : v_black);
					util::log(std::to_string(next_col));

					// If we can move front and there is not piece there, yes we add it in pos list.
					if (chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && concurrent_piece.type == chess::piece::EMPTY) {
						pos_list.push_back(pos);
					}

					util::log(std::to_string(concurrent_piece.type));
					break;

					// Move right.
					chess::matrix::move(next_row, 1);

					// If some piece is in up-right to piece, we can kill so add in pos list.
					if (chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color == (white ? chess::color::BLACK : chess::color::WHITE)) {
						pos_list.push_back(pos);
					}

					// Now move left (considering it was right, so double move to left).	
					chess::matrix::move(next_row, -2);

					// Samething but now for up-left
					if (chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color == (white ? chess::color::BLACK : chess::color::WHITE)) {
						pos_list.push_back(pos);
					}

					util::log("Collection end. Color (white): " + std::to_string(white));

					// End.
					break;
				} else {
					break;
				}
			}
		}

		static uint8_t find(uint8_t row, uint8_t col) {
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
	};

	static piece_data map[64];
	static std::vector<entity_piece> loaded_entity_list;
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

	float x, y, w, h, dx, dy;
	uint8_t alpha;
	
	static util::texture texture;
	static uint8_t white_dock; 

	util::color color_white = util::color(255, 255, 255, 255), color_black = util::color(0, 0, 0, 255);

	static void set_piece(entity_piece &entity, uint8_t type) {
		if (type == chess::piece::PAWN) {
			entity.set(chess::render::pawn);
		} else if (type == chess::piece::TOWER) {
			entity.set(chess::render::tower);
		} else if (type == chess::piece::HORSE) {
			entity.set(chess::render::horse);
		} else if (type == chess::piece::BISHOP) {
			entity.set(chess::render::bishop);
		} else if (type == chess::piece::KING) {
			entity.set(chess::render::king);
		} else if (type == chess::piece::QUEEN) {
			entity.set(chess::render::queen);
		}
	}

	static uint8_t find(const std::string &letter, uint8_t slot) {
		uint8_t index = chess::OUT_RANGE;

		for (piece_data &tiles : chess::map) {
		}

		return index;
	}

	static void move(entity_piece &entity, uint8_t pos) {
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
	}

	void init() {
		this->refresh();

		if (!util::file::read_texture(texture, GL_RGBA, "data/textures/chess-symbols.png")) {
			util::log("Could not load chess symbols texture.");
		}

		util::log("Chess game core initialized.");
	}

	void new_game() {
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

	void end_game() {
		this->gaming = false;
	}

	void refresh() {
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

	void on_event(SDL_Event &sdl_event) {
		switch (sdl_event.type) {
			case SDL_MOUSEBUTTONDOWN: {
				// Clean before use.
				this->possible.clear();

				float x = sdl_event.motion.x;
				float y = sdl_event.motion.y;

				this->over = (x > this->x && y > this->y && x < this->x + this->w && y < this->y + this->h);

				if (!this->over) {
					break;
				}

				this->focused = chess::OUT_RANGE;
				entity_piece entity;

				for (uint8_t i = 0; i < chess::loaded_entity_list.size(); i++) {
					entity = chess::loaded_entity_list.at(i);

					if (x > entity.x && y > entity.y && x < entity.x + entity.w && y < entity.y + entity.h) {
						this->start.x = entity.x;
						this->start.y = entity.y;
						this->start.w = entity.w;
						this->start.h = entity.h;

						this->start_pos = true;
						this->end_pos = false;

						chess::matrix::from(entity.x - this->y, entity.y - this->y, this->matrix_pos[0], this->matrix_pos[1]);
						chess::matrix::possible(this->possible, entity.piece_slot.type, entity.color_factory, this->matrix_pos[0], this->matrix_pos[1]);
						break;
					}
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

	void on_render(float render_ticks) {
		if (!this->gaming) {
			return;
		}

		tessellator::fx(fx_manager::light_specular_fx);

		// Set the specular light position.
		fx_manager::light_specular_fx.use();
		fx_manager::light_specular_fx.set_float("x", this->x + (this->w / 2));
		fx_manager::light_specular_fx.set_float("y", this->y + (this->h / 2));

		util::render::shape(0, 0, screen_w, screen_h, util::color(255, 0, 0, 255));
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
			entity.on_render(1.0f);
		}

		if (this->over) {
			util::render::shape_outline(this->hovered.x, this->hovered.y, this->hovered.w, this->hovered.h, 1.0f, util::color(0, 255, 0, 50));
		}

		if (this->dragging) {
			util::render::shape(this->hovered.x, this->hovered.y, this->hovered.w, this->hovered.h, util::color(0, 0, 255, 50));
		}
	}
};

util::texture chess::texture = util::texture();
uint8_t chess::white_dock = chess::TOP;
float chess::square_size = 30;

piece_data chess::render::queen  = piece_data();
piece_data chess::render::bishop = piece_data();
piece_data chess::render::pawn   = piece_data();
piece_data chess::render::tower  = piece_data();
piece_data chess::render::horse  = piece_data();
piece_data chess::render::king   = piece_data();

piece_data chess::map[64] = {};
std::vector<entity_piece> chess::loaded_entity_list = {};

chess chess_game;

void on_event(SDL_Event &sdl_event) {
	switch (sdl_event.type) {
		case SDL_MOUSEMOTION: {
			// Set the fx stuff.
			fx_manager::mouse_outline_fx.use();
			fx_manager::mouse_outline_fx.set_float("mx", sdl_event.motion.x);
			fx_manager::mouse_outline_fx.set_float("my", sdl_event.motion.y);
			fx_manager::mouse_outline_fx.end();

			break;
		}
	}
}

void on_update(uint64_t delta) {

}

void on_render(float render_ticks) {
	// Prepare context of us tools.
	shader_manager::context();
	fx_manager::context();

	// Clear buffers.
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	chess_game.on_render(render_ticks);
}

int main(int argv, char** argc) {
	bool running = true;

	// Init SDL2 and create window.
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* sdl_win = SDL_CreateWindow("Dungeon Of SDL2.", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_w, screen_h, SDL_WINDOW_OPENGL);
	
	// Init SDL image.
	IMG_Init(IMG_INIT_PNG);

	// Create OpenGL context in SDL2.
	SDL_GLContext sdl_gl_context = SDL_GL_CreateContext(sdl_win);
	SDL_Event sdl_event;

	// Init glew.
	glewExperimental = GL_TRUE;
	if (glewInit()) {
		util::log("Nooo :(, glew does not want to init.");
		running = false;
	}

	// Setup OpenGL core.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// Set anti-allasing.linda
	glEnable(GL_MULTISAMPLE);

	// Clock variables.
	uint64_t previous_ticks = SDL_GetTicks64();
	uint64_t current_ticks  = SDL_GetTicks64();
	uint64_t interval       = 1000 / 60;
	uint64_t delta          = 0;
	uint32_t elapsed_frames = 0;
	uint32_t fps            = 0;

	// Test.
	//const std::string file = read_file("oi_sou.linda");
	//util::log(file);

	// Logs.
	util::log("SDL2 and OpenGL setup successfully.");
	std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// Init the tools.
	fx_manager::init();
	tessellator::init();

	// Init chess stuff.
	chess_game.alpha = 200;
	chess::square_size = 60;
	chess_game.init();
	chess::render::init();

	// Based on metrics of chess table, set new pos and refresh.
	chess_game.x = (screen_w / 2) - (chess_game.w / 2);
	chess_game.y = (screen_h / 2) - (chess_game.h / 2);
	chess_game.refresh();
	chess_game.new_game();

	while (running) {
		while (SDL_PollEvent(&sdl_event)) {
			if (sdl_event.type == SDL_QUIT) {
				running = false;
				break;
			}

			on_event(sdl_event);
			chess_game.on_event(sdl_event);
		}

		// Verify the ticks difference.
		current_ticks = SDL_GetTicks64() - previous_ticks;

		// If the difference is not less than interval we call this tick.
		if (current_ticks > interval) {
			// Update delta and set the previous based in actual ticks.
			previous_ticks = SDL_GetTicks64();
			delta += current_ticks;

			// Update and render.
			on_update(delta);
			on_render((float) current_ticks);

			// Count elapsed frames after render.
			elapsed_frames++;

			// Reset delta if needed.
			if (delta > 1000) {
				fps = elapsed_frames;
				elapsed_frames = 0;
				delta = 0;
			}

			// Swap buffers.
			SDL_GL_SwapWindow(sdl_win);
		}
	}

	SDL_Quit();
	IMG_Quit();

	util::log("Shutdown complete!");

	return 0;
}