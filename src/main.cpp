#include "includes.h"
#include "util.h"
#include "tessellator.h"

struct entity_piece {
	float x, y;
	float previous_x, previous_y;

	bool alive;

	uint8_t death_master;
	uint8_t texture_id;

	void on_render(float render_ticks) {
	};
};

struct piece_data {
	float x, y, w, h;
	uint8_t type;
};

struct chess {
	enum piece {
		PAWN, TOWER,
		HORSE, BISHOP,
		QUEEN, KING, EMPTY
	};

	enum color {
		WHITE, BLACK
	};

	piece_data map[64];
	piece_data hovered;

	bool over;
	float x, y, w, h, square_size;

	uint8_t alpha;
	util::color color_white = util::color(255, 255, 255, 255), color_black = util::color(0, 0, 0, 255);

	void init() {
		this->refresh();
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
 
				this->map[iterator++] = pd;
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

					for (piece_data &pieces : this->map) {
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
		this->color_white.a = alpha;
		this->color_black.a = alpha;

		color tile_color = color::WHITE;
		uint8_t t = 0;

		for (piece_data &pieces : this->map) {
			// Swap color.
			tile_color = tile_color == color::WHITE ? color::BLACK : color::WHITE;

			if (t == 8) {
				tile_color = tile_color == color::WHITE ? color::BLACK : color::WHITE;
				t = 0;
			}

			// Render the tiles.
			util::render::shape(pieces.x, pieces.y, pieces.w, pieces.h, tile_color == color::WHITE ? this->color_white : this->color_black);
			
			tessellator::fx(fx_manager::mouse_outline_fx);
			util::render::shape_outline(pieces.x, pieces.y, pieces.w, pieces.h, 1.0f, util::color(0, 0, 255, 200));
			tessellator::fx();
			
			t++;
		}

		if (this->over) {
			tessellator::fx(fx_manager::mouse_outline_fx);
			util::render::shape_outline(hovered.x, hovered.y, hovered.w, hovered.h, 5.0f, util::color(0, 255, 0, 50));
			tessellator::fx();
		}
	}
};

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

	tessellator::fx(fx_manager::mouse_outline_fx);
	util::render::shape(10, 10, 200, 200, util::color(255, 255, 255, 255));
	tessellator::fx();

	tessellator::start(GL_LINE_STRIP, 2);
	tessellator::vertex(200, 200, 0);
	tessellator::color(255, 255, 255, 255);
	tessellator::vertex(200, 600, 0);
	tessellator::color(255, 255, 255, 255);
	tessellator::draw();
}

int main(int argv, char** argc) {
	bool running = true;
	uint32_t screen_w = 800, screen_h = 600;

	// Init SDL2 and create window.
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* sdl_win = SDL_CreateWindow("Dungeon Of SDL2.", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_w, screen_h, SDL_WINDOW_OPENGL);
	
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
	util::log("Shutdown complete!");

	return 0;
}