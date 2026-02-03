#ifndef _MY_SIMPLE_SPRITE_H
#define _MY_SIMPLE_SPRITE_H

//-----------------------------------------------------------------------------
// MySimpleSprite.h
// A very slight modification of SimpleSprite which makes it
// more accessible to being extended.
//-----------------------------------------------------------------------------

#include <freeglut_config.h>
#include <map>
#include <vector>
#include <string>

namespace Engine
{
    //-----------------------------------------------------------------------------
    // MySimpleSprite
    //-----------------------------------------------------------------------------
    class MySimpleSprite
    {
    public:
        MySimpleSprite(const char* fileName, const unsigned int nColumns = 1, const unsigned int nRows = 1);

        virtual void Update(const float dt);
        virtual void Draw();
        virtual void SetPosition(const float x, const float y) { m_xpos = x; m_ypos = y; }
        virtual void SetAngle(const float a) { m_angle = a; }
        virtual void SetScale(const float s) { m_scale = s >= 0.0f ? s : 0.0f; }
        virtual void SetFrame(const unsigned int f);
        virtual void SetAnimation(const int id);
        virtual void SetAnimation(const int id, const bool playFromBeginning);
        virtual void GetPosition(float& x, float& y) const { x = m_xpos; y = m_ypos; }
        virtual float GetWidth() const { return m_width; }
        virtual float GetHeight() const { return m_height; }
        virtual float GetAngle() const { return m_angle; }
        virtual float GetScale() const { return m_scale; }
        virtual unsigned int GetFrame() const { return m_frame; }
        virtual void SetColor(const float r, const float g, const float b) { m_red = r; m_green = g; m_blue = b; }
    
        // Note: speed must be > 0, frames must have size >= 1, id must be unique among animations
        virtual void CreateAnimation(const unsigned int id, const float speed, const std::vector<int>& frames)
        {
            sAnimation anim;
            anim.m_id = id;
            anim.m_speed = speed;
            anim.m_frames = frames;
            m_animations.push_back(anim);
        };
    
    protected:
        void CalculateUVs();
        GLuint m_texture;
        float m_xpos = 0.0f;
        float m_ypos = 0.0f;
        float m_width = 0.0f;
        float m_height = 0.0f;
        int   m_texWidth = 0;
        int   m_texHeight = 0;
        float m_angle = 0.0f;
        float m_scale = 1.0f;
        float m_points[8];
        float m_uvcoords[8];
        unsigned int m_frame;
        unsigned int m_nColumns;
        unsigned int m_nRows;
        float m_red = 1.0f;
        float m_green = 1.0f;
        float m_blue = 1.0f;
        int   m_currentAnim = -1;
        float m_animTime = 0.0f;
    
        struct sAnimation
        {
            unsigned int m_id = 0;
            float m_speed = 0.0f;
            std::vector<int> m_frames;
        };
        std::vector<sAnimation> m_animations;
    
        // Texture management.
        struct sTextureDef
        {
            unsigned int m_width;
            unsigned int m_height;
            GLuint m_textureID;
        };
        virtual bool LoadTexture(const std::string& filename);
        
        static std::map<std::string, sTextureDef> m_textures;
    };
}

#endif // _MY_SIMPLE_SPRITE_H