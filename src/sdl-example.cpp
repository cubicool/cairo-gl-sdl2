#include "common/SDL2.hpp"
#include "common/timediff.hpp"

#include <cairo.h>
#include <cairo-gl.h>
#include <GL/glu.h>

// Size of the surface.
const unsigned int WIDTH = 512;
const unsigned int HEIGHT = 512;

inline void draw(cairo_surface_t* surface) {
	static double s = 1.0;

	cairo_t* cr = cairo_create(surface);

	cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
	cairo_paint(cr);
	cairo_translate(cr, WIDTH / 2, HEIGHT / 2);
	cairo_scale(cr, s, s);
	cairo_arc(cr, 0.0, 0.0, WIDTH / 4, 0.0, 2.0 * 3.14159);
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
	cairo_fill(cr);
	cairo_surface_flush(surface);
	cairo_destroy(cr);

	s += 1.0 / 180.0;

	if(s >= 2.0) s = 1.0;
}

int main(int argc, char** argv) {
	SDL2Window window;

	if(!window.init(WIDTH, HEIGHT)) {
		std::cerr << "Couldn't initialize SDL2 window; fatal." << std::endl;

		return 2;
	}

	if(window.makeCurrent()) {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glViewport(0.0, 0.0, WIDTH, HEIGHT);
		glClearColor(0.0, 0.1, 0.2, 1.0);
	}

	cairo_device_t* device = cairo_glx_device_create(
		window.getDisplay(),
		reinterpret_cast<GLXContext>(window.getCairoContext())
	);

	if(!device) {
		std::cerr << "Couldn't create device; fatal." << std::endl;

		return 3;
	}

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

	cairo_surface_t* surface = cairo_gl_surface_create_for_texture(
		device,
		CAIRO_CONTENT_COLOR_ALPHA,
		texture,
		WIDTH,
		HEIGHT
	);

	if(!surface) {
		std::cerr << "Couldn't create surface; fatal." << std::endl;

		return 4;
	}

	unsigned long frames = 0;
	unsigned long cairoTime = 0;
	unsigned long sdlTime = 0;

	auto drawStart = std::chrono::system_clock::now();

	window.main([&]() {
		auto start = std::chrono::system_clock::now();

		if(window.makeCairoCurrent()) {
			draw(surface);

			cairo_gl_surface_swapbuffers(surface);
		}

		cairoTime += timediff(start);

		start = std::chrono::system_clock::now();

		if(window.makeCurrent()) {
			int x = 0;
			int y = 0;
			int width = 512;
			int height = 512;

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glBegin(GL_QUADS);

			// Bottom-Left
			glTexCoord2i(0, 1);
			glVertex2i(x, y);

			// Upper-Left
			glTexCoord2i(0, 0);
			glVertex2i(x, y + height);

			// Upper-Right
			glTexCoord2i(1, 0);
			glVertex2i(x + width, y + height);

			// Bottom-Right
			glTexCoord2i(1, 1);
			glVertex2i(x + width, y);

			glEnd();
		}

		sdlTime += timediff(start);

		frames++;
	});

	auto fps = frames / (timediff(drawStart) / 1000);

	std::cout << "FPS: " << fps << std::endl;
	std::cout << "Cairo average time: " << cairoTime / frames << "ms" << std::endl;
	std::cout << "SDL2 average time: " << sdlTime / frames << "ms" << std::endl;

	cairo_surface_destroy(surface);
	cairo_device_destroy(device);

	window.deinit();

	return 0;
}

