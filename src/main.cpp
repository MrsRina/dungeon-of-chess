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

	piece_data piece_slot;
	util::texture texture;

	entity_piece(float pos_x, float pos_y, float metrics_w, float metrics_h, uint8_t color, piece_data &_piece_data, &util::texture _texture) {
		this->x = pos_x;
		this->y = pos_y;
		this->w = metrics_w;
		this->h = metrics_h;
		this->piece_slot = piece_data;
		this->texture = _texture;
	}

	void set(piece_data &_piece_data) {
		this->piece_slot = piece_data;
	}

	void on_render(float render_ticks) {
		// Draw the piece.
		util::render::shape_texture(this->x, this->y, this->w, this->h, this->piece_slot.x, this->piece_slot.y, this->piece_slot.w, this->piece_slot.h, this->texture);
	};
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
	};

	struct matrix {
		static void move(uint8_t &not_matrix_pos, uint8_t velocity) {
			if (not_matrix_pos + velocity < 8) {
				not_matrix_pos += velocity;
			}
		}

		static bool get(piece_data &_piece_data, uint8_t pos) {
			if (pos > 64) {
				return false;
			}

			_piece_data = util::map[pos];

			if (_piece_data == NULL) {
				return false;
			}

			return true;
		}

		static uint8_t* possible(uint8_t type, uint8_t color, uint8_t row, uint8_t col) {
			bool white = color;
			bool real = false;

			piece_data concurrent_piece;

			uint8_t pos = 0;
			uint8_t next_row = row;
			uint8_t next_col = col;

			// Stack of pos possible in this calc.
			uint8_t i;
			uint8_t[64] pos_list;

			// Velocity based on side positions.
			int8_t v_white = chess::white_dock == chess::TOP ?  1 : -1;
			int8_t v_black = chess::white_dock == chess::TOP ? -1 :  1;

			// We have a loop with 256 ticks.
			// 64 * 4 = 256
			for (uint8_t ticks = 0; ticks < 256; ticks++) {
				white = chess::render::get_color();

				if (type == chess::piece::PAWN) {
					// Move the piece to front (based in velocity).
					chess::matrix::move(next_col, white ? v_white : v_black);

					// If we can move front and there is not piece there, yes we add it in pos list.
					if (chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && concurrent_piece.type == chess::piece::EMPTY) {
						pos_list[i++] = pos;
					}

					// Move right.
					chess::matrix::move(next_row, 1);

					// If some piece is in up-right to piece, we can kill so add in pos list.
					if (chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color == (white ? chess::color::BLACK : chess::color::WHITE)) {
						pos_list[i++] = pos;
					}

					// Now move left (considering it was right, so double move to left).	
					chess::matrix::move(next_row, -2);

					// Samething but now for up-left
					if (chess::matrix::get(concurrent_piece, pos = chess::matrix::find(next_row, next_col)) && concurrent_piece.type != chess::piece::EMPTY && concurrent_piece.color == (white ? chess::color::BLACK : chess::color::WHITE)) {
						stack_pos[i++] = pos;
					}
				}
			}

			return pos_list;
		}

		static uint8_t find(uint8_t row, uint8_t col) {
			uint8_t last_jump = 0;

			uint8_t concurrent_jump = 0;
			uint8_t concurrent_rows = 0;
			uint8_t concurrent_cols = 0;

			for (uint8_t jumps = 1; jumps < 64; jumps++) {
				if (concurrent_rows == row && concurrent_cols = col) {
					return jumps;
				}

				concurrent_jump = i - last_jump;

				if (concurrent_jump == 7) {
					last_jump = jumps;

					if (concurrent_cols != 7) {
						concurrent_cols++;
					}

					concurrent_rows = 0;
				}

				concurrent_rows++;
			}

			return chess::OUT_RANGE;
		}
	};

	static piece_data map[64];
	static std::vector<entity_piece> loaded_entity_list;
	
	piece_data hovered;

	bool over;
	bool gaming;

	float x, y, w, h, square_size;
	uint8_t alpha;
	
	static util::texture texture;
	static uint8_t white_dock; 

	util::color color_white = util::color(255, 255, 255, 255), color_black = util::color(0, 0, 0, 255);

	static void set_color(entity_piece &entity, chess::color piece_color) {
		float height = texture.height / 2;
		entity.piece_slot.y = piece_color == chess::color::WHITE ? height : 0.0f;
	}

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
		} else if (type == chesS::piece::QUEEN) {
			entity.set(type == chess::render::queen);
		}
	}

	static uint8_t find(const std::string &letter, uint8_t slot) {
		uint8_t index = chess::OUT_RANGE;

		for (piece_data &tiles : chess::map) {

		}

		return index;
	}

	static void reset_piece_place(entity_piece &entity) {
		chess::color color = chess::render::get_color(entity);
		chess::piece type = entity.piece_slot.type;

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
		} else if (type == chesS::piece::QUEEN) {
			entity.set(type == chess::render::queen);
		}

		if (color == chess::color::WHITE) {

		}
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

		uint8_t not_infantry = {piece::TOWER, piece::HORSE, piece::BISHOP, piece::KING, piece::QUEEN, pience::BISHOP, piece::HORSE, bishop::TOWER};
		
		uint8_t pos_x = chess::white_dock == chess::TOP ? 0 : 64;
		uint8_t pos_y = chess::white_dock == chess::TOP ? 0 : 64;

		loaded_entity_list.push_back(entity_piece(this->x + (i++ * this->square_size),
												  this->y + (i * this->square_size),
												  this->square_size, this->square_size,
												  chess::color::WHITE, chess::piece::pawn));

		loaded_entity_list.push_back(entity_piece(this->x + (i++ * this->square_size),
												  this->y + (i * this->square_size),
												  this->square_size, this->square_size,
												  chess::color::WHITE, chess::piece::pawn));
	}

	void end_game() {
		this->gaming = false;
	}

	void refresh() {
		this->w = 8 * this->square_size;
		this->h = 8 * this->square_size;

		uint8_t iterator = 0;

		// s = sync.
		float sx = this->x, sy = this->y;

		for (uint8_t i = 0; i < 8; i++) {
			for (uint8_t j = 0; j < 8; j++) {
				piece_data pd;

				pd.x = sx;
				pd.y = sy;

				pd.w = square_size;
				pd.h = square_size;
 
				chess::map[iterator++] = pd;
				sx += this->square_size;
			}

			sx = this->x;
			sy += this->square_size;
		}
	}

	void on_event(SDL_Event &sdl_event) {
		switch (sdl_event.type) {
			case SDL_MOUSEMOTION: {
				float x = sdl_event.motion.x;
				float y = sdl_event.motion.y;

				this->over = false;

				if (x > this->x && y > this->y && x < this->x + this->w && y < this->y + this->h) {
					this->over = true;

					for (piece_data &pieces : chess::map) {
						if (x > pieces.x && y > pieces.y && x < pieces.x + pieces.w && y < pieces.y + pieces.h) {
							this->hovered = pieces;
							break;
						}
					}
				}

				break;
			}
		}
	}

	void on_render(float render_ticks) {
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

		for (piece_data &pieces : chess::map) {
			// Swap color.
			tile_color = tile_color == color::WHITE ? color::BLACK : color::WHITE;

			if (t == 8) {
				tile_color = tile_color == color::WHITE ? color::BLACK : color::WHITE;
				t = 0;
			}

			// Render the tiles.
			util::render::shape(pieces.x, pieces.y, pieces.w, pieces.h, tile_color == color::WHITE ? this->color_white : this->color_black);
			t++;
		}

		if (this->over) {
			util::render::shape_outline(hovered.x, hovered.y, hovered.w, hovered.h, 1.0f, util::color(0, 255, 0, 50));
		}
	}
};

util::texture chess::texture = util::texture();
uint8_t chess::white_dock = chess::TOP;

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
	chess_game.square_size = 30;
	chess_game.alpha = 200;
	chess_game.init();
	chess::render::init();

	// Based on metrics of chess table, set new pos and refresh.
	chess_game.x = (screen_w / 2) - (chess_game.w / 2);
	chess_game.y = (screen_h / 2) - (chess_game.h / 2);
	chess_game.refresh();

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