#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include <cstdint>
#include <freeglut_config.h>
#include <string>

#include <engine/Spatial.h>

namespace Engine
{
    class Color;
    class Scene;

    namespace Graphics
    {
        inline float resMult = 2.0f;

        // Draw a 2D line
        void drawLine(const Vec2i &start, const Vec2i &end, const Color &col);
        void drawLine(const Linei &line, const Color &col);

        void drawLineCam(class Scene *scene, const Vec2i &start, const Vec2i &end, const Color &col);
        void drawLineCam(class Scene *scene, const Linei &line, const Color &col);
        
        // Draw a 2D triangle
        void drawTriangle(const Vec2i p1, const Vec2i p2, const Vec2i p3, const Color &col, bool isWireframe = false);
        void drawTriangleCam(class Scene *scene, const Vec2i p1, const Vec2i p2, const Vec2i p3, const Color &col, bool isWireframe = false);

        // Draw a 2D rectangle
        void drawRect(const Recti &rect, const Color &col);
        void drawRectCam(class Scene *scene, const Recti &rect, const Color &col);
        
        // Draw a filled 2D rectangle
        void drawRectFilled(const Recti &rect, const Color &col);
        void drawRectFilledCam(class Scene *scene, const Recti &rect, const Color &col);
        
        // Draw text to the screen ('font' expects a GLUT font)
        void drawText(const Vec2i &position, const std::string &str, const Color &col, void *font = GLUT_BITMAP_HELVETICA_18);
        void drawTextCam(class Scene *scene, const Vec2i &position, const std::string &str, const Color &col, void *font = GLUT_BITMAP_HELVETICA_18);
        
        // Window resize callback
        void windowResized(int w, int h);
    }

    class Color
    {
    public:
        uint8_t r, g, b, a;

        constexpr Color()
            : r(0), g(0), b(0), a(255) {}

        constexpr Color(uint8_t r, uint8_t g, uint8_t b)
            : r(r), g(g), b(b), a(255) {}

        constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
            : r(r), g(g), b(b), a(a) {}

        constexpr static Color fromFloat(float r, float g, float b, float a = 1.0f)
        {
            Color color;
            color.r = static_cast<uint8_t>(r * 255.0f);
            color.g = static_cast<uint8_t>(g * 255.0f);
            color.b = static_cast<uint8_t>(b * 255.0f);
            color.a = static_cast<uint8_t>(a * 255.0f);
            return color;
        }

        constexpr Color lerp(const Color &rhs, float fac) const
        {
            return Color(
                Math::lerp(r, rhs.r, fac),
                Math::lerp(g, rhs.g, fac),
                Math::lerp(b, rhs.b, fac),
                Math::lerp(a, rhs.a, fac));
        }

        constexpr float r_f32() const
        {
            return static_cast<float>(r) / 255.0f;
        }
        constexpr float g_f32() const
        {
            return static_cast<float>(g) / 255.0f;
        }
        constexpr float b_f32() const
        {
            return static_cast<float>(b) / 255.0f;
        }
        constexpr float a_f32() const
        {
            return static_cast<float>(a) / 255.0f;
        }

        // Doesn't multiply alpha
        constexpr Color operator*(float f) const
        {
            return Color(
                r * f,
                g * f,
                b * f,
                a);
        }

        // Doesn't multiply alpha
        constexpr Color &operator*=(const float rhs)
        {
            r *= rhs;
            g *= rhs;
            b *= rhs;
            return *this;
        }

        constexpr bool operator==(const Color &rhs) const
        {
            return (
                r == rhs.r &&
                g == rhs.g &&
                b == rhs.b &&
                a == rhs.a);
        }

        constexpr bool operator!=(const Color &rhs) const
        {
            return !(*this == rhs);
        }

        static const Color white;
        static const Color black;
        static const Color red;
        static const Color green;
        static const Color blue;
        static const Color vsBlue;
    };

    inline const Color Color::white = Color(255, 255, 255);
    inline const Color Color::black = Color(0, 0, 0);
    inline const Color Color::red = Color(255, 0, 0);
    inline const Color Color::green = Color(0, 255, 0);
    inline const Color Color::blue = Color(0, 0, 255);
    inline const Color Color::vsBlue = Color(93, 107, 153);
}

#endif // _GRAPHICS_H