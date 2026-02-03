#ifndef _GAME_H
#define _GAME_H

#include <freeglut_config.h>

#include <engine/Spatial.h>
#include <engine/Sprite.h>
#include <engine/Tritone.h>
#include <engine/Input.h>
#include <engine/Camera.h>

#include <stack>
#include <memory>
#include <filesystem>

namespace Engine
{
    class Scene;
    
    // Can't really do anything with this...
    struct GameConfig
    {
        Vec2i windowSize = Vec2i(640, 480);
        float windowScale = 1.0f;

        GLuint glMinFilter = GL_NEAREST;
        GLuint glMagFilter = GL_NEAREST;
        bool genMipmaps = false;

        bool debugMode = true;
    };

    class Game
    {
    private:
        // The Game's global config
        GameConfig m_config;
    
        // The main stack of scenes
        std::stack<std::unique_ptr<Scene>> mSceneStack;

        // Path to the data folder
        std::filesystem::path mDataPath;

    public:
        // -- Managers --
        // Tritone playback engine
        TriTone::Playback m_tritonePlayer;
        
        // Main input manager
        Input::Manager m_input;

        Game(const GameConfig &config);

        // Call once before adding any scenes to the game
        void init();
        // Update current scene
        void update(const float dt);
        // Draw current scene
        void draw();
        // Free game
        void destroyed();

        void handleGlobalControls();

        // -- Scene management --
        Scene *getScene();
        void popScene();

        template <class T>
        void pushScene()
        {
            Graphics::resMult = 2.0f;

            mSceneStack.push(std::make_unique<T>());
            Scene *scene = mSceneStack.top().get();

            // Give scene a reference to the game class
            scene->m_game = this;

            // Setup camera object
            scene->m_camera.m_size = getScreenSize();

            // Initialize scene
            scene->init();
        }

        template <class T>
        void setScene()
        {
            while (mSceneStack.size() > 1)
            {
                popScene();
            }
            pushScene<T>();
        }

        // Create a new sprite ('relativePath' starts at the game's 'data' directory)
        Sprite createSprite(const std::filesystem::path &relativePath, const unsigned int nColumns = 1, const unsigned int nRows = 1);
        
        // Get an absolute path to the data folder
        std::filesystem::path getDataPath() const;

        // Get screen size
        Vec2i getScreenSize();
        int getScreenWidth();
        int getScreenHeight();

        // Get the game configuration
        const GameConfig &getConfig() const { return m_config; }

        void restartScene();
    };
}

#endif // _GAME_H
