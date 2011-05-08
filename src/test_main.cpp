#include "util3d/Matrix.hpp"
#include "util3d/Vector.hpp"
#include "util3d/MatrixTransform.hpp"
#include "util3d/gl/VertexArrayObject.hpp"
#include "util3d/gl/BufferObject.hpp"
#include "util3d/gl/Shader.hpp"
#include "util3d/gl/ShaderProgram.hpp"
#include "util3d/gl/Texture.hpp"
#include "image/ImageLoader.hpp"
#include "util2d/SpriteBatch.hpp"
#include "util2d/Tilemap.hpp"

#include <iostream>
#include <fstream>

#include "util3d/gl3w.hpp"

//#define GLFW_GL3_H
#include <GL/glfw.h>

using namespace math;

mat4 screen_transform;

void APIENTRY debug_callback(GLenum /*source*/, GLenum /*type*/, GLuint /*id*/, GLenum /*severity*/, GLsizei /*length*/, const GLchar* message, GLvoid* /*userParam*/)
{
	std::cout << message << std::endl;
	exit(2);
}

void sprite_test()
{
	image::Image img;
	{
		std::ifstream f("data/ship-no-outline.png", std::ios::in | std::ios::binary);
		image::Image::loadPNGFileRGBA8(img, f);
	}
	image::preMultiplyAlpha(img);

	util2d::SpriteBatch::initialize_shared();

	util2d::SpriteBatch spr_batch;
	const int num_spr = 16;
	util2d::Sprite* spr[num_spr];
	const float pi = 3.14159265f;
	float rot[num_spr];
	signed char spdx[num_spr];
	signed char spdy[num_spr];
	for (int i = 0; i < num_spr; ++i) {
		spr[i] = spr_batch.newSprite();
		rot[i] = rand() / (float)RAND_MAX * 2*pi;
		spdx[i] = rand() % 2;
		spdy[i] = rand() % 2;

		if (rand() & 1) spdx[i] = -1;
		if (rand() & 1) spdy[i] = -1;

		spr[i]->x = rand() % 799;
		spr[i]->y = rand() % 599;
		spr[i]->img_x = spr[i]->img_y = 0;
		spr[i]->img_w = 3;
		spr[i]->img_h = 4;
		spr[i]->r = 255;
		spr[i]->g = 255;
		spr[i]->b = 255;
		spr[i]->a = 255;

		spr[i]->transform = mat_transform::identity<2>();
	}


	gl::Texture tex;
	glActiveTexture(GL_TEXTURE0);
	tex.bind(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	tex.width = img.getWidth();
	tex.height = img.getHeight();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getWidth(), img.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getData());

	spr_batch.setTexture(&tex);

	bool running = true;

	glClearColor(1.f, .5f, 0.f, 1.f);

	while (running) {
		double time_start = glfwGetTime();

		for (int i = 0; i < num_spr; ++i) {
			rot[i] += 0.01f;

			spr[i]->x += spdx[i];
			spr[i]->y += spdy[i];

			if (spr[i]->x >= 799 || spr[i]->x < 0) spdx[i] = -spdx[i];
			if (spr[i]->y >= 599 || spr[i]->y < 0) spdy[i] = -spdy[i];

			const float c = std::cos(rot[i]);
			const float s = std::sin(rot[i]);

			spr[i]->transform(0,0) = c;
			spr[i]->transform(1,0) = s;
			spr[i]->transform(0,1) = -s;
			spr[i]->transform(1,1) = c;

			mat2 scale_mat = {{4.f, 0.f, 0.f, 4.f}};

			spr[i]->transform = spr[i]->transform * scale_mat;
		}

		double time_gpu = glfwGetTime();

		glClear(GL_COLOR_BUFFER_BIT);

		spr_batch.draw();

		std::cout << "\r" << "Total: " << (glfwGetTime() - time_start) * 1000.0 << "ms" << "      Draw: " << (glfwGetTime() - time_gpu) * 1000.0 << "ms        " << std::flush;

		glfwSwapBuffers();

		running = glfwGetWindowParam(GLFW_OPENED) != 0;
	}
}

void tilemap_test()
{
	image::Image img;
	{
		std::ifstream f("tileset.png", std::ios::in | std::ios::binary);
		image::Image::loadPNGFileRGBA8(img, f);
	}

	util2d::Tilemap::initialize_shared();

	gl::Texture tex;
	glActiveTexture(GL_TEXTURE0);
	tex.bind(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	tex.width = img.getWidth();
	tex.height = img.getHeight();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getWidth(), img.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getData());

	unsigned short map[20*15];
	for (unsigned short i = 0; i < 20*15; ++i) {
		map[i] = i;
	}

	util2d::Tilemap tilemap(0, 0, 320, 240);
	tilemap.setTexture(&tex);
	tilemap.setTilemap(map, 20, 15);
	tilemap.offx = tilemap.offy = 0;

	bool running = true;

	glClearColor(.2f, .2f, .2f, 1.f);

	while (running) {
		double time_start = glfwGetTime();

		glClear(GL_COLOR_BUFFER_BIT);

		tilemap.offx += 5;
		tilemap.offy -= 3;
		tilemap.draw();

		std::cout << "\r" << (glfwGetTime() - time_start) * 1000.0 << "ms" << std::flush;

		glfwSwapBuffers();

		running = glfwGetWindowParam(GLFW_OPENED) != 0;
	}
}

int main(int /*argc*/, char** /*argv*/)
{
	if (glfwInit() != GL_TRUE) {
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return 1;
	}

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (glfwOpenWindow(800, 600, 8, 8, 8, 8, 24, 8, GLFW_WINDOW) != GL_TRUE) {
		std::cerr << "Failed to open window." << std::endl;
		return 1;
	}

	if (gl3wInit() != 0) {
		std::cerr << "Failed to initialize gl3w." << std::endl;
		return 1;
	} else if (!gl3wIsSupported(3, 3)) {
		std::cerr << "OpenGL 3.3 not supported." << std::endl;
		return 1;
	}

	if (glDebugMessageCallbackARB) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
		glDebugMessageCallbackARB(debug_callback, 0);
	}

	{
		vec3 s = {{2.f/800.f, -2.f/600.f, 1.f}};
		vec3 t = {{-1.f, 1.f, 0.f}};
		screen_transform = mat_transform::translate(t) * mat_transform::scale(s);
	}

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	sprite_test();
	//tilemap_test();

	glfwTerminate();

	return 0;
}
