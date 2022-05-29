#include "includes.h"
#include "util.h"

void on_update(uint64_t delta) {

}

void on_render(float render_ticks) {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

int main(int argv, char** argc) {
	bool running = true;

	// Init glew.
	glewExperimental = GL_TRUE;
	glewInit();

	// Init SDL2 and create window.
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* sdl_win = SDL_CreateWindow("Dungeon Of SDL2.", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
	// Setup OpenGL core.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// Create OpenGL context in SDL2.
	SDL_GLContext sdl_gl_context = SDL_GL_CreateContext(sdl_win);
	SDL_Event sdl_event;

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
	util::log("Program initialized successfully.");

	while (running) {
		while (SDL_PollEvent(&sdl_event)) {
			if (sdl_event.type == SDL_QUIT) {
				running = false;
			}
		}

		// Verify the ticks difference.
		current_ticks = SDL_GetTicks64() - previous_ticks;

		// If the difference is not less than interval we call this tick.
		if (current_ticks > interval) {
			// Update delta and set the previous based in actual ticks.
			previous_ticks = SDL_GetTicks64();
			delta += current_ticks;

			// Reset delta if needed.
			if (delta > 1000) {
				fps = elapsed_frames;
				elapsed_frames = 0;
				delta = 0;
			}

			// Update and render.
			on_update(delta);
			on_render((float) current_ticks);

			// Count elapsed frames after render.
			elapsed_frames++;

			// Swap buffers.
			SDL_GL_SwapWindow(sdl_win);
		}
	}

	return 0;
}