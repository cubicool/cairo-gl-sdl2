#ifndef SDL2_HPP
#define SDL2_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <tuple>
#include <iostream>
#include <memory>

// TODO: This will require more, eventually.
typedef std::function<void ()> SDL2DrawCallback;

class SDL2Window {
public:
	typedef std::tuple<int, int> size_type;

	SDL2Window():
	_window(nullptr),
	_context(nullptr),
	_cairoContext(nullptr) {
	}

	bool init(int width, int height, unsigned int flags=0) {
		if(SDL_Init(SDL_INIT_VIDEO)) {
			std::cerr << "SDL_Init failed!" << std::endl;

			return false;
		}

		_window = SDL_CreateWindow(
			"Cairo+OpenGL+SDL2",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width,
			height,
			SDL_WINDOW_OPENGL | flags
		);

		if(!_window) {
			std::cerr << "SDL_CreateWindow failed!" << std::endl;

			return false;
		}

		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

		_context = SDL_GL_CreateContext(_window);
		_cairoContext = SDL_GL_CreateContext(_window);

		if(!_context || !_cairoContext) {
			std::cerr << "SDL_GL_CreateContext failed." << std::endl;

			return false;
		}

		SDL_VERSION(&_info.version);

		if(!SDL_GetWindowWMInfo(_window, &_info)) {
			std::cerr << "SDL_GetWindowWMInfofailed." << std::endl;
			std::cerr << SDL_GetError() << std::endl;

			return false;
		}

		return true;
	}

	void deinit() {
		SDL_GL_MakeCurrent(_window, nullptr);
		SDL_GL_DeleteContext(_context);
		SDL_GL_DeleteContext(_cairoContext);
		SDL_DestroyWindow(_window);
		SDL_Quit();
	}

	double main(SDL2DrawCallback draw) {
		double frames = 0.0;
		bool running = true;
		Uint32 start = SDL_GetTicks();

		SDL_Event event;

		while(running) {
			while(SDL_PollEvent(&event)) {
				if(_mainQuit(event)) running = false;
			}

			draw();

			SDL_GL_SwapWindow(_window);

			frames += 1.0;
    	}

		return frames / static_cast<double>(SDL_GetTicks() - start);
	}

	size_type getSize() const {
		int w = 0;
		int h = 0;

		SDL_GetWindowSize(_window, &w, &h);

		return std::make_tuple(w, h);
	}

	int getWidth() const {
		int w = 0;

		std::tie(w, std::ignore) = getSize();

		return w;
	}

	int getHeight() const {
		int h = 0;

		std::tie(std::ignore, h) = getSize();

		return h;
	}

	bool isFullscreen() const {
		return SDL_GetWindowFlags(_window) & SDL_WINDOW_FULLSCREEN;
	}

	void setFullscreen(bool go_fullscreen) const {
		SDL_SetWindowFullscreen(_window, go_fullscreen ? 1 : 0);
	}

	const char* getTitle() const {
		return SDL_GetWindowTitle(_window);
	}

	void setTitle(const std::string& title) {
		SDL_SetWindowTitle(_window, title.c_str());
	}

	Display* getDisplay() {
		return _info.info.x11.display;
	};

	SDL_GLContext getContext() {
		return _context;
	}

	SDL_GLContext getCairoContext() {
		return _cairoContext;
	}

	bool makeCurrent() {
		SDL_GL_MakeCurrent(_window, nullptr);

		return !SDL_GL_MakeCurrent(_window, _context);
	}

	bool makeCairoCurrent() {
		SDL_GL_MakeCurrent(_window, nullptr);

		return !SDL_GL_MakeCurrent(_window, _cairoContext);
	}

protected:
	inline bool _mainQuit(const SDL_Event& event) const {
		if(event.type == SDL_QUIT || (
			event.type == SDL_KEYUP &&
			event.key.keysym.sym == SDLK_ESCAPE
		)) return true;

		return false;
	}

	SDL_Window* _window;
	SDL_GLContext _context;
	SDL_GLContext _cairoContext;
	SDL_SysWMinfo _info;
};

#endif

