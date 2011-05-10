#ifndef COMBOSHOOTER_UTIL2D_SPRITEBATCH_H
#define COMBOSHOOTER_UTIL2D_SPRITEBATCH_H

namespace gl {
	class Texture;
}

#include "Sprite.hpp"
#include "../util3d/gl/VertexArrayObject.hpp"
#include "../util3d/gl/BufferObject.hpp"

#include <list>

namespace engine { class GraphicsManager; }

namespace util2d {

class SpriteBatch
{
public:
	SpriteBatch(engine::GraphicsManager& graphics_manager);

	Sprite* newSprite();
	void draw() const;

	void setTexture(gl::Texture* tex);
	const gl::Texture* getTexture() const;

	static void initialize_shared();
	static void deinitialize_shared();

private:
	void remove(Sprite& spr);

	engine::GraphicsManager& graphics_manager;

	gl::Texture* texture;
	std::list<Sprite> sprites;

	gl::VertexArrayObject spr_vao;
	gl::BufferObject spr_vbo;

	unsigned int buffer_size; // In sprites

	friend class Sprite;
};

} // namespace util2d

#endif // COMBOSHOOTER_UTIL2D_SPRITEBATCH_H
