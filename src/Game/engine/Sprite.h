#ifndef _SPRITE_H
#define _SPRITE_H

#include "internal/MySimpleSprite.h"
#include <engine/Spatial.h>
#include <engine/Graphics.h>

#include <filesystem>

namespace Engine
{
    class Game;
    
    // An extension of CSimpleSprite with support for
    // - Non-uniform scaling
    // - Scaling/rotating about an origin
    // - Using my own spatial structures
    class Sprite : public MySimpleSprite
    {
    private:
        Vec2i m_origin = Vec2i(0, 0);
        Vec2f m_scale2d = Vec2f(1, 1);

        Color m_color = Color(255, 255, 255, 255);

        // Pointer to game class
        class Game *m_game = nullptr;
    public:
        enum class Origin
        {
            TopLeft,
            TopMiddle,
            TopRight,

            Left,
            Middle,
            Right,

            BottomLeft,
            BottomMiddle,
            BottomRight,

            Custom,

            Count
        };

        Sprite();
        Sprite(class Game *game, const std::filesystem::path &path, const unsigned int nColumns = 1, const unsigned int nRows = 1);
        Sprite(class Game *game, const char *fileName, const unsigned int nColumns = 1, const unsigned int nRows = 1);

        void SetOrigin(Origin origin);
        void SetOrigin(const Vec2i &origin);
        Vec2i GetOrigin() const;

        void SetAnimationInstant(const int id, int frame = 0);

        void SetPosition(const float x, const float y) override;
        void SetPosition(const Vec2f &newPos);
        Vec2f GetPosition() const;

        float GetWidth() const override;
        float GetHeight() const override; 
        Vec2f GetSize() const;

        void SetScale(const float s) override;
        void SetScale(const float x, const float y);
        void SetScale(const Vec2f &scale);
        Vec2f GetScale2D() const;

        void SetColor(const float r, const float g, const float b) override;
        void SetColor(const Color &color);

        Color GetColor() const;

        // Unfortunate, but our new scaling system supports non-uniform scaling
        float GetScale() = delete;

        // Update the sprite's animation
        void Update(const float dt) override;
        // Draw the sprite
        void Draw() override;
        // Draw with useful options
        void DrawEx(const Vec2f &pos, const Vec2f &scale = Vec2f(1, 1), const int anim = -1, const Color &color = Color(255, 255, 255));
        void DrawExCam(const Vec2f &pos, const Vec2f &scale = Vec2f(1, 1), const int anim = -1, const Color &color = Color(255, 255, 255));

    protected:
        bool LoadTexture(const std::string& filename) override;
    };
}

#endif // _SPRITE_H