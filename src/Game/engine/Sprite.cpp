#include "Sprite.h"
#include <iostream>

#include <app.h>
#include <engine/DebugConsole.h>
#include <engine/Game.h>
#include <engine/ecs/Scene.h>
#include <vendor/stb_image/stb_image.h>

using namespace Engine;
namespace fs = std::filesystem;

namespace
{
	template <class T>
	Vec2<T> flippedCoords(const Vec2<T> &vec)
	{
		return Vec2<T>(vec.x, APP_VIRTUAL_HEIGHT - vec.y);
	}
}

// Force a default constructor...
// NOTE: This seems to be alright to do. CSimpleSprite doesn't do anything if it can't find a file
Sprite::Sprite()
	: MySimpleSprite("", 1, 1)
{
}

Sprite::Sprite(Game *game, const std::filesystem::path &path, const unsigned int nColumns, const unsigned int nRows)
	: Sprite(game, path.string().c_str(), nColumns, nRows)
{
}

// This constructor is called by above
Sprite::Sprite(Game *game, const char *fileName, const unsigned int nColumns, const unsigned int nRows)
	: MySimpleSprite("", nColumns, nRows), m_game(game)
{
	LB_ASSERT(fs::exists(fileName), "Sprite does not exist.");

	if (LoadTexture(fileName))
	{
		CalculateUVs();
		m_points[0] = -(m_width / 2.0f);
		m_points[1] = -(m_height / 2.0f);
		m_points[2] = m_width / 2.0f;
		m_points[3] = -(m_height / 2.0f);
		m_points[4] = m_width / 2.0f;
		m_points[5] = m_height / 2.0f;
		m_points[6] = -(m_width / 2.0f);
		m_points[7] = m_height / 2.0f;
	}
}

void Sprite::Update(const float dt)
{
	MySimpleSprite::Update(dt);
}

void Sprite::Draw()
{
	{
#if APP_USE_VIRTUAL_RES
		float scalex = (1.0f / APP_VIRTUAL_WIDTH) * 2.0f;
		float scaley = (1.0f / APP_VIRTUAL_HEIGHT) * 2.0f;

		// float scalex = (m_scale2d.x / APP_VIRTUAL_WIDTH) * 2.0f;
		// float scaley = (m_scale2d.y / APP_VIRTUAL_HEIGHT) * 2.0f;
#else
		float scalex = m_scale2d.x;
		float scaley = m_scale2d.y;
#endif
		// Use origin offset
		float halfWidth = GetWidth() / 2;
		float halfHeight = GetHeight() / 2;
		float screenWidth = m_game->getScreenWidth();
		float screenHeight = m_game->getScreenHeight();

		Vec2i scaledOrigin = Vec2f(m_origin) * Vec2f(m_scale2d);

		float xTopLeft = m_xpos + halfWidth;
		float yTopLeft = screenHeight - m_ypos - halfHeight;

		float x = xTopLeft - m_origin.x;
		float y = yTopLeft + m_origin.y;

		float xOrigin = (screenWidth / 2) - m_origin.x + halfWidth;
		float yOrigin = (screenHeight / 2) + m_origin.y - halfHeight;

#if APP_USE_VIRTUAL_RES
		APP_VIRTUAL_TO_NATIVE_COORDS(x, y);
		APP_VIRTUAL_TO_NATIVE_COORDS(xOrigin, yOrigin);

#endif
		glPushMatrix();

		{
			// Translate to sprite's draw position
			glTranslatef(x, y, 0.0f);
			
			// Move back from origin
			glTranslated(-xOrigin, -yOrigin, 0);
			
			// Fix messed up rotation on non-uniformly scaled sprites
			// NOTE: Will probably only work if screen_width > screen_height
			glScaled(1.0f, (float)APP_VIRTUAL_WIDTH / (float) APP_VIRTUAL_HEIGHT, 1.0f);
			glRotatef(m_angle * 180 / PI, 0.0f, 0.0f, 1.0f);
			glScaled(1.0f, (float)APP_VIRTUAL_HEIGHT / (float) APP_VIRTUAL_WIDTH, 1.0f);
			
			// Scale using sprite's scale factor
			glScalef(m_scale2d.x, m_scale2d.y, 1.0);
			
			// Move to origin
			glTranslated(xOrigin, yOrigin, 0);
	
			// Scale so 1pxSprite == 1pxFrameBuffer
			glScalef(scalex, scaley, 1.0f);
		}

		// Set color
		glColor4f(m_color.r_f32(), m_color.g_f32(), m_color.b_f32(), m_color.a_f32());

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_texture);

		glBegin(GL_QUADS);
		for (unsigned int i = 0; i < 8; i += 2)
		{
			glTexCoord2f(m_uvcoords[i], m_uvcoords[i + 1]);
			glVertex2f(m_points[i], m_points[i + 1]);
		}

		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
	}
}

void Sprite::DrawEx(const Vec2f &pos, const Vec2f &scale, const int anim, const Color &color)
{
	SetPosition(pos);
	SetScale(scale);

	if (anim >= 0)
	{
		SetAnimationInstant(anim);
	}

	m_color = color;

	Draw();
}

void Engine::Sprite::DrawExCam(const Vec2f &pos, const Vec2f &scale, const int anim, const Color &color)
{
	Vec2i camPos = m_game->getScene()->m_camera.m_pos;
	DrawEx(pos - camPos, scale, anim, color);
}

bool Sprite::LoadTexture(const std::string &filename)
{
	// Don't try to do anything if we were instantiated properly
	if (m_game == nullptr)
	{
		return false;
	}

	// Exit if we already have the texture cached
	if (m_textures.find(filename) != m_textures.end())
	{
		sTextureDef &texDef = m_textures[filename];
		m_texture = texDef.m_textureID;
		m_texWidth = texDef.m_width;
		m_texHeight = texDef.m_height;
		return true;
	}

	// -- Load a new texture from disk --
	
	int channels;
	unsigned char *imageData = stbi_load(filename.c_str(), &m_texWidth, &m_texHeight, &channels, 4);

	// Get settings from game config
	GameConfig config = m_game->getConfig();
	GLuint minFilter = config.glMinFilter;
	GLuint magFilter = config.glMagFilter;
	bool genMipMaps = config.genMipmaps;

	GLuint texture = 0;
	if (imageData)
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (genMipMaps)
		{
			gluBuild2DMipmaps(GL_TEXTURE_2D, 4, m_texWidth, m_texHeight, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texWidth, m_texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
		}

		stbi_image_free(imageData);

		sTextureDef textureDef = {(unsigned int)m_texWidth, (unsigned int)m_texHeight, texture};
		m_textures[filename] = textureDef;
		m_texture = texture;
		return true;
	}

	return false;
}

void Sprite::SetOrigin(Origin origin)
{
	Vec2i full = Vec2i(GetWidth(), GetHeight());
	Vec2i half = full / 2;

	switch (origin)
	{
	case Origin::TopLeft:
		m_origin = Vec2i(0, 0);
		break;
	case Origin::TopMiddle:
		m_origin = Vec2i(half.x, 0);
		break;
	case Origin::TopRight:
		m_origin = Vec2i(full.x, 0);
		break;

	case Origin::Left:
		m_origin = Vec2i(0, half.y);
		break;
	case Origin::Middle:
		m_origin = Vec2i(half.x, half.y);
		break;
	case Origin::Right:
		m_origin = Vec2i(full.x, half.y);
		break;

	case Origin::BottomLeft:
		m_origin = Vec2i(0, full.y);
		break;
	case Origin::BottomMiddle:
		m_origin = Vec2i(half.x, full.y);
		break;
	case Origin::BottomRight:
		m_origin = Vec2i(full.x, full.y);
		break;

	default:
		break;
	}
}

void Sprite::SetOrigin(const Engine::Vec2i &origin)
{
	m_origin = origin;
}

Vec2i Sprite::GetOrigin() const
{
	return m_origin;
}

void Sprite::SetAnimationInstant(const int id, int frame)
{
	SetAnimation(id, true);

	if (m_animations.size() > 0)
	{
		sAnimation &anim = m_animations[m_currentAnim];
		SetFrame(anim.m_frames[frame]);
	}
}

void Sprite::SetPosition(const Engine::Vec2f &newPos)
{
	MySimpleSprite::SetPosition(newPos.x, newPos.y);
}

void Sprite::SetPosition(const float x, const float y)
{
	MySimpleSprite::SetPosition(x, y);
}

Vec2f Sprite::GetPosition() const
{
	Vec2f pos;
	MySimpleSprite::GetPosition(pos.x, pos.y);
	return pos;
}

float Engine::Sprite::GetWidth() const
{
	return MySimpleSprite::GetWidth();
}

float Engine::Sprite::GetHeight() const
{
	return MySimpleSprite::GetHeight();
}

Vec2f Sprite::GetSize() const
{
	return Vec2f(GetWidth(), GetHeight());
}

void Sprite::SetScale(const float s)
{
	SetScale(s, s);
}

void Sprite::SetScale(const float x, const float y)
{
	m_scale2d.x = x;
	m_scale2d.y = y;
}

void Sprite::SetScale(const Engine::Vec2f &scale)
{
	m_scale2d = scale;
}

Engine::Vec2f Sprite::GetScale2D() const
{
	return m_scale2d;
}

void Sprite::SetColor(const float r, const float g, const float b)
{
	m_color = Color::fromFloat(r, g, b);
}

void Sprite::SetColor(const Color &color)
{
	m_color = color;
}

Color Engine::Sprite::GetColor() const
{
	return m_color;
}
