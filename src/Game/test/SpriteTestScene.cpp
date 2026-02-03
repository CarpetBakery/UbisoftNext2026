#include "SpriteTestScene.h"

#include <freeglut_config.h>
#include <app.h>

#include <engine/Input.h>
#include <engine/Sprite.h>
#include <engine/Ecs.h>

using namespace Engine;
using namespace GS;

namespace
{
    float xScale = 1.0f;
    float yScale = 1.0f;
    float rotation = 0.0f;

    bool swapSong = false;

    struct SpriteTest
    {
        bool followMouse = false;
    };

    class SpriteTestSystem : public System
    {
        Sprite sprite;
        std::string originTypeStr = "";

    public:
        void init() override
        {
            Signature sig;
            sig.set(m_scene->getComponentType<SpriteTest>());

            setup(0, Type::Draw, sig);

            sprite = m_game->createSprite("gfx/tritone/piano.png");
            sprite.SetOrigin(Sprite::Origin::Middle);
            sprite.CreateAnimation(0, 1, {0});
            sprite.SetPosition(Vec2i(0, 0));

            originTypeStr = originToString(Sprite::Origin::TopLeft);
        }

        void update() override
        {
            // Clear screen
            glClearColor(0.6f, 0.6f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (auto const ent : m_entities)
            {
                SpriteTest &spriteTest = m_scene->getComponent<SpriteTest>(ent);

                if (App::IsKeyPressed(App::KEY_R))
                {
                    rotation = 0;
                    xScale = 1.0f;
                    yScale = 1.0f;
                }

                testRotation(spriteTest);
                testScale(spriteTest);
                // testInput(spriteTest);
                testOrigin(spriteTest.followMouse);

                testLineIntersect();

                if (m_game->m_input.keyPressed(App::KEY_SPACE))
                {
                    if (m_game->m_tritonePlayer.isPlaying())
                    {
                        m_game->m_tritonePlayer.pause();
                    }
                    else
                    {
                        if (swapSong)
                        {
                            m_game->m_tritonePlayer.loadAndPlay("voltest.tri");
                        }
                        else
                        {
                            m_game->m_tritonePlayer.loadAndPlay("silly.tri");
                        }
                        swapSong = !swapSong;
                    }
                }

                // if (m_game->m_input.keyPressed(App::KEY_E))
                // {
                //     m_game->pushScene<TritoneEditorScene>();
                // }

                break;
            }
        }

        void testRotation(SpriteTest &spriteTest)
        {
            float amt = 0.01f;
            bool pressed = false;
            if (App::IsKeyPressed(App::KEY_A))
            {
                rotation -= amt;
                pressed = true;
            }
            if (App::IsKeyPressed(App::KEY_D))
            {
                rotation += amt;
                pressed = true;
            }
            if (pressed)
            {
                printf("Rotation: %f\n", rotation);
            }
        }

        void testScale(SpriteTest &spriteTest)
        {
            if (App::IsKeyPressed(App::KEY_LEFT))
            {
                xScale -= 0.1;
            }
            if (App::IsKeyPressed(App::KEY_RIGHT))
            {
                xScale += 0.1;
            }
            if (App::IsKeyPressed(App::KEY_DOWN))
            {
                yScale -= 0.1;
            }
            if (App::IsKeyPressed(App::KEY_UP))
            {
                yScale += 0.1;
            }
        }

        void testInput(SpriteTest &spriteTest)
        {
            if (m_game->m_input.mLeftPressed())
            {
                spriteTest.followMouse = true;
                printf("Pressed left\n");
            }
            if (m_game->m_input.mRightPressed())
            {
                spriteTest.followMouse = false;
                printf("Pressed right\n");
            }
            if (m_game->m_input.mMiddlePressed())
            {
                spriteTest.followMouse = false;
                printf("Pressed middle\n");
            }
            for (int i = 0; i < m_game->m_input.mWheelUp(); i++)
            {
                printf("Mouse wheel Up\n");
            }
            for (int i = 0; i < m_game->m_input.mWheelDown(); i++)
            {
                printf("Mouse wheel Down\n");
            }
        }

        void testOrigin(bool followMouse)
        {
            // Change origin
            if (App::IsKeyPressed(App::KEY_1))
            {
                sprite.SetOrigin(Sprite::Origin::TopLeft);
                originTypeStr = "TopLeft";
            }
            else if (App::IsKeyPressed(App::KEY_2))
            {
                sprite.SetOrigin(Sprite::Origin::TopMiddle);
                originTypeStr = "TopMiddle";
            }
            else if (App::IsKeyPressed(App::KEY_3))
            {
                sprite.SetOrigin(Sprite::Origin::TopRight);
                originTypeStr = "TopRight";
            }
            else if (App::IsKeyPressed(App::KEY_4))
            {
                sprite.SetOrigin(Sprite::Origin::Left);
                originTypeStr = "Left";
            }
            else if (App::IsKeyPressed(App::KEY_5))
            {
                sprite.SetOrigin(Sprite::Origin::Middle);
                originTypeStr = "Middle";
            }
            else if (App::IsKeyPressed(App::KEY_6))
            {
                sprite.SetOrigin(Sprite::Origin::Right);
                originTypeStr = "Right";
            }
            else if (App::IsKeyPressed(App::KEY_7))
            {
                sprite.SetOrigin(Sprite::Origin::BottomLeft);
                originTypeStr = "BottomLeft";
            }
            else if (App::IsKeyPressed(App::KEY_8))
            {
                sprite.SetOrigin(Sprite::Origin::BottomMiddle);
                originTypeStr = "BottomMiddle";
            }
            else if (App::IsKeyPressed(App::KEY_9))
            {
                sprite.SetOrigin(Sprite::Origin::BottomRight);
                originTypeStr = "BottomRight";
            }

            std::string originStr = "Origin Pos: " + sprite.GetOrigin().toString() + "\n";
            originStr += "Origin type: " + originTypeStr;

            Vec2i printPos = Vec2i(2, 20);
            printPos.y = m_game->getScreenSize().y - printPos.y;
            App::Print(printPos.x, printPos.y, originStr.c_str());

            if (!followMouse)
            {
                sprite.SetPosition(m_game->getScreenSize() / 2);
            }
            else
            {
                sprite.SetPosition(m_game->m_input.mousePos());
            }

            sprite.Update(99);
            sprite.SetScale(xScale, yScale);
            sprite.SetAngle(rotation);
            sprite.Draw();

            App::DrawLine(APP_VIRTUAL_WIDTH / 2, APP_VIRTUAL_HEIGHT / 2,
                          APP_VIRTUAL_WIDTH / 2 + 1, APP_VIRTUAL_HEIGHT / 2 + 1,
                          1.0f, 0.0f, 0.0f);
        }

        std::string originToString(Sprite::Origin origin)
        {
            switch (origin)
            {
            case Sprite::Origin::TopLeft:
                return "TopLeft";
                break;
            case Sprite::Origin::TopMiddle:
                return "TopMiddle";
                break;
            case Sprite::Origin::TopRight:
                return "TopRight";
                break;
            case Sprite::Origin::Left:
                return "Left";
                break;
            case Sprite::Origin::Middle:
                return "Middle";
                break;
            case Sprite::Origin::Right:
                return "Right";
                break;
            case Sprite::Origin::BottomLeft:
                return "BottomLeft";
                break;
            case Sprite::Origin::BottomMiddle:
                return "BottomMiddle";
                break;
            case Sprite::Origin::BottomRight:
                return "BottomRight";
                break;

            default:
                return "";
            }
        }

        void testLineIntersect()
        {
            Linei l1 = Linei(Vec2i(20, 20), Vec2i(70, 400));
            Linei l2 = Linei(Vec2i(140, 20), m_game->m_input.mousePos());
            Recti r = Recti(Vec2i(200, 200), Vec2i(200, 100));

            Color col = Color::blue;

            if (l1.intersects(l2))
            {
                col = Color::red;
            }
            Graphics::drawLine(l1, col);
            Graphics::drawLine(l2, col);

            // if (l2.intersects(r))
            if (r.intersects(l2))
            {
                col = Color::green;
            }

            Graphics::drawRect(r, col);
        }
    };
}

void SpriteTestScene::init()
{
    printf("-- Sprite test scene --\n");

    // Register components
    registerComponent<SpriteTest>();

    // Register Systems
    registerSystem<SpriteTestSystem>();

    // Create entities
    auto ent = createEntity();
    addComponent(ent, SpriteTest());

    // Test rect
    Rectf rect = Rect(0, 0, m_game->getScreenWidth(), m_game->getScreenHeight());
    printf("Rect: %s\n", rect.toString().c_str());
    printf("topLeft: %s\n", rect.topLeft().toString().c_str());
    printf("topMiddle: %s\n", rect.topMiddle().toString().c_str());
    printf("topRight: %s\n", rect.topRight().toString().c_str());
    printf("left: %s\n", rect.left().toString().c_str());
    printf("middle: %s\n", rect.middle().toString().c_str());
    printf("right: %s\n", rect.right().toString().c_str());
    printf("bottomLeft: %s\n", rect.bottomLeft().toString().c_str());
    printf("bottomMiddle: %s\n", rect.bottomMiddle().toString().c_str());
    printf("bottomRight: %s\n", rect.bottomRight().toString().c_str());

    // Play some music!
    m_game->m_tritonePlayer.loadAndPlay("voltest.tri");
}