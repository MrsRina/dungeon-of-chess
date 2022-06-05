#include "includes.h"
#include "util.h"
#include "chess.h"
#include "tessellator.h"

uint32_t screen_w = 800;
uint32_t screen_h = 600;

chess chess_game;
SDL_Window* sdl_win;

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

		case SDL_WINDOWEVENT: {
			if (sdl_event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				int32_t w, h;
				SDL_GetWindowSize(sdl_win, &w, &h);

				screen_w = (uint32_t) w;
				screen_h = (uint32_t) h;

				chess_game.set_pos((screen_w / 2) - (chess_game.w / 2), (screen_h / 2) - (chess_game.h / 2));
				chess_game.refresh(sdl_win);
			}

			break;
		}
	}
}

void on_update(uint64_t delta) {
	if (!chess_game.gaming) {
		chess_game.new_game();
	}

	chess_game.on_update(delta);
}

void on_render(float render_ticks) {
	// Set viewport.
	glViewport(0, 0, screen_w, screen_h);

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
	sdl_win = SDL_CreateWindow("Dungeon of SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_w, screen_h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	
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
	uint64_t the_fps_config = 60;
	uint64_t interval       = 1000 / the_fps_config;
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
	chess_game.gamemode_godmode = false;
	chess_game.gamemode_cycle = true;

	// After setup settings we init main core and render core.
	chess::square_size = 60;
	chess::white_dock = chess::TOP;

	chess_game.init(sdl_win);
	chess::render::init();

	// Based on metrics of chess table, set new pos and refresh.
	chess_game.set_pos((screen_w / 2) - (chess_game.w / 2), (screen_h / 2) - (chess_game.h / 2));
	chess_game.refresh(sdl_win);

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

			// Segment of update.
			on_update(delta);

			// Segment of render.
			on_render((float) current_ticks / the_fps_config);

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