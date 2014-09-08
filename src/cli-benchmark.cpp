#include "common/SDL2.hpp"
#include "common/timediff.hpp"

#include <cairo.h>
#include <cairo-gl.h>

// Size of the surface.
const unsigned int WIDTH = 512;
const unsigned int HEIGHT = 512;

inline void draw(cairo_surface_t* surface) {
	cairo_t* cr = cairo_create(surface);

	cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
	cairo_paint(cr);
	cairo_translate(cr, WIDTH / 2, HEIGHT / 2);
	cairo_arc(cr, 0.0, 0.0, WIDTH / 4, 0.0, 2.0 * 3.14159);
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
	cairo_fill(cr);

	// TODO: Determine the importance of this call, how it can be avoided,
	// and how it affects performance!
	// cairo_surface_flush(surface);

	cairo_destroy(cr);
}

int main(int argc, char** argv) {
	if(argc != 3) {
		std::cerr
			<< "Usage: " << argv[0]
			<< " num_draws [image | gl | gl_texture]" << std::endl
		;

		return 1;
	}

	SDL2Window window;

	if(!window.init(WIDTH, HEIGHT, SDL_WINDOW_HIDDEN)) {
		std::cerr << "Couldn't initialize SDL2 window; fatal." << std::endl;

		return 2;
	}

    cairo_device_t* device = cairo_glx_device_create(
		window.getDisplay(),
		reinterpret_cast<GLXContext>(window.getCairoContext())
	);

	if(!device) {
		std::cerr << "Couldn't create device; fatal." << std::endl;

		return 3;
	}

	// Variable initialization. In the future, I'll need to setup
	// the OpenGL texture to use here as well.
	auto num_draws = std::atoi(argv[1]);
	auto method = std::string(argv[2]);
	cairo_surface_t* surface = nullptr;

	if(method == "image" ) surface = cairo_image_surface_create(
		CAIRO_FORMAT_ARGB32,
		WIDTH,
		HEIGHT
	);

	else if(method == "gl") surface = cairo_gl_surface_create(
		device,
		CAIRO_CONTENT_COLOR_ALPHA,
		WIDTH,
		HEIGHT
	);

	else if(method == "gl_texture") {
		GLuint texture = 0;

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,
			WIDTH,
			HEIGHT,
			0,
			GL_BGRA_EXT,
			GL_UNSIGNED_BYTE,
			nullptr
		);

		surface = cairo_gl_surface_create_for_texture(
			device,
			CAIRO_CONTENT_COLOR_ALPHA,
			texture,
			WIDTH,
			HEIGHT
		);
	}

	else {
		std::cerr << "Unknown surface type '" << method << "'; fatal." << std::endl;

		return 4;
	}

	if(!surface) {
		std::cerr << "Couldn't create surface; fatal." << std::endl;

		return 5;
	}

	std::cout << "Performing " << num_draws << " iterations: " << std::flush;

	auto start = std::chrono::system_clock::now();
	auto last_tick = 0;

	for(auto i = 0; i < num_draws; i++) {
		draw(surface);

		// This is a completely wretched way of doing a progress meter,
		// but it's the best I'm willing to do for now.
		double pct = (static_cast<double>(i) / static_cast<double>(num_draws)) * 100.0;

		if(pct >= last_tick + 10.0) {
			std::cout << "+" << std::flush;

			last_tick = pct;
		}
	}

	std::cout << " done! (" << timediff(start) << "ms)" << std::endl;

	cairo_surface_destroy(surface);
	cairo_device_destroy(device);

	window.deinit();

	return 0;
}

