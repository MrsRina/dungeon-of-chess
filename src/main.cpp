#include "includes.h"
#include "util.h"
#include "tessellator.h"

void on_update(uint64_t delta) {

}

void on_render(float render_ticks) {
	float viewport[4];
	glGetFloatv(GL_VIEWPORT, viewport);

	float proj_mat[16];
	util::math::ortho2d(proj_mat, 0.0f, viewport[2], viewport[3], 0.0f);

	// Set the proj matrix in shader.
	tessellator::context(proj_mat);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	float x = 10;
	float y = 10;
	float w = 200;
	float h = 200;

	float vertex[18] = {
		x, y, 0,
		x, y + h, 0,
		x + w, y + h, 0,
		x + w, y + h, 0,
		x + w, y, 0,
		x, y, 0
	};

	float color[24] = {
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1
	};

	tessellator::start(GL_TRIANGLES, 6);
	tessellator::draw(18, 24, vertex, color);
}

int main(int argv, char** argc) {
	bool running = true;

	// Init SDL2 and create window.
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* sdl_win = SDL_CreateWindow("Dungeon Of SDL2.", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
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
	tessellator::init();

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