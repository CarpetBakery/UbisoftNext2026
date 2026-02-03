#include "Graphics.h"

#include <app.h>
#include <freeglut_config.h>

#include <engine/ecs/Scene.h>

using namespace Engine;

void Graphics::drawLine(const Vec2i &start, const Vec2i &end, const Color &col)
{
    // TODO: Everything with APP_VIRTUAL_HEIGHT will break
    // the moment I start supporting dynamically changing resolution
    App::DrawLine(start.x, APP_VIRTUAL_HEIGHT - start.y,
                  end.x, APP_VIRTUAL_HEIGHT - end.y,
                  col.r / 255.0f,
                  col.g / 255.0f,
                  col.b / 255.0f);
}

void Graphics::drawLine(const Linei &line, const Color &col)
{
    drawLine(line.a, line.b, col);
}

void Graphics::drawLineCam(Scene *scene, const Vec2i &start, const Vec2i &end, const Color &col)
{
    Vec2i camPos = scene->m_camera.m_pos;
    drawLine(start - camPos, end - camPos,col);
}

void Graphics::drawLineCam(Scene *scene, const Linei &line, const Color &col)
{
    Vec2i camPos = scene->m_camera.m_pos;
    drawLine(line - camPos, col);
}

void Graphics::drawTriangle(const Vec2i p1, const Vec2i p2, const Vec2i p3, const Color &col, bool isWireframe)
{
    App::DrawTriangle(
        p1.x, APP_VIRTUAL_HEIGHT - p1.y, 0, 1,
        p2.x, APP_VIRTUAL_HEIGHT - p2.y, 0, 1,
        p3.x, APP_VIRTUAL_HEIGHT - p3.y, 0, 1,
        col.r_f32(), col.g_f32(), col.b_f32(),
        col.r_f32(), col.g_f32(), col.b_f32(),
        col.r_f32(), col.g_f32(), col.b_f32(),
        isWireframe);
}

void Graphics::drawTriangleCam(Scene *scene, const Vec2i p1, const Vec2i p2, const Vec2i p3, const Color &col, bool isWireframe)
{
    Vec2i camPos = scene->m_camera.m_pos;
    drawTriangle(
        p1 - camPos,
        p2 - camPos,
        p3 - camPos,
        col,
        isWireframe);
}

void Graphics::drawRect(const Recti &rect, const Color &col)
{
    Vec2i off = Vec2i(1, 0);

    // Top
    drawLine(rect.topLeft() - off, rect.topRight(), col);
    // Bottom
    drawLine(rect.bottomLeft(), rect.bottomRight(), col);
    // Left
    drawLine(rect.topLeft(), rect.bottomLeft(), col);
    // Right
    drawLine(rect.topRight(), rect.bottomRight(), col);
}

void Graphics::drawRectCam(Scene *scene, const Recti &rect, const Color &col)
{
    Vec2i camPos = scene->m_camera.m_pos;
    drawRect(rect - camPos, col);
}

void Graphics::drawRectFilled(const Recti &rect, const Color &col)
{
    drawTriangle(
        rect.topLeft(),
        rect.bottomRight(),
        rect.bottomLeft(),
        col);
    drawTriangle(
        rect.topLeft(),
        rect.topRight(),
        rect.bottomRight(),
        col);
}

void Graphics::drawRectFilledCam(Scene *scene, const Recti &rect, const Color &col)
{
    Vec2i camPos = scene->m_camera.m_pos;
    drawRectFilled(rect - camPos, col);
}

void Graphics::drawText(const Vec2i &position, const std::string &str, const Color &col, void *font)
{
    App::Print(position.x, APP_VIRTUAL_HEIGHT - position.y,
               str.c_str(),
               col.r_f32(), col.g_f32(), col.b_f32(),
               font);
}

void Graphics::drawTextCam(Scene *scene, const Vec2i &position, const std::string &str, const Color &col, void *font)
{
    Vec2i camPos = scene->m_camera.m_pos;
    drawText(position - camPos, str, col, font);
}

void Graphics::windowResized(int w, int h)
{
    printf("Window resized to: %d, %d\n", w, h);

    // TODO: Look at glutInitWindow source and see what they did
    // Default functionality
    glViewport(0, 0, w, h);

    // If we don't want the window to be resized, you can call this function inside the callback
    glutReshapeWindow(APP_VIRTUAL_WIDTH * 2, APP_VIRTUAL_HEIGHT * 2);
    glutPositionWindow(APP_VIRTUAL_WIDTH / 2, APP_VIRTUAL_HEIGHT / 2);
}