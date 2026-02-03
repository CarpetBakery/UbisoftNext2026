#ifndef _CONTENT_H
#define _CONTENT_H

#include <engine/Sprite.h>
#include <filesystem>

namespace Engine
{
    class Game;
}

class Content
{
private: 
    inline static class Engine::Game *m_game = nullptr;

public:
    static void load(class Engine::Game *game);
    static void unload();

    // -- Sprites -------------------------------------------
    // -- Game --
    static inline Engine::Sprite sprPlayer;
    static inline Engine::Sprite sprShadow;
    static inline Engine::Sprite sprCoin;
    static inline Engine::Sprite sprSkull;

    static inline Engine::Sprite sprGrappleArrow;
    static inline Engine::Sprite sprGrappleRope;
    static inline Engine::Sprite sprGrappleLaser;
    
    static inline Engine::Sprite sprCursorInner;
    static inline Engine::Sprite sprCursorOuter;

    static inline Engine::Sprite sprCeilingHook;
    static inline Engine::Sprite sprCeilingHookEffect;

    static inline Engine::Sprite sprStartBg;
    static inline Engine::Sprite sprMainBg;
    static inline Engine::Sprite sprMainBgBehind;


    // Effects
    static inline Engine::Sprite sprSmoke1;
    static inline Engine::Sprite sprSmoke2;

    // -- TriTone editor --
    static inline Engine::Sprite sprNote;
    static inline const int aNoteLeft = 0;
    static inline const int aNoteMid = 1;
    static inline const int aNoteRight = 2;
    
    static inline Engine::Sprite sprEventMark;
    
    static inline Engine::Sprite sprPiano;
    
    static inline Engine::Sprite sprOctMarker;
    
    static inline Engine::Sprite sprBg;
    static inline const int aPianoBgLight = 0;
    static inline const int aPianoBgDark = 1;
    
    static inline Engine::Sprite sprBotpanelBg;
    
    static inline Engine::Sprite sprBotpanelMain;
    
    static inline Engine::Sprite sprBotpanelLabelPan;
    
    static inline Engine::Sprite sprBotpanelLabelVelocity;

    static inline Engine::Sprite sprVelocityButton;
    
    static inline Engine::Sprite sprPanButton;
    
    static inline Engine::Sprite sprScrollButtons;
    static inline const int aScrollButtonLeft = 0;
    static inline const int aScrollButtonRight = 1;

    static inline Engine::Sprite sprScrollBar;
    static inline const int aScrollbarLeft = 0;
    static inline const int aScrollbarMid = 1;
    static inline const int aScrollbarRight = 2;

    static inline Engine::Sprite sprScrollBg;
    static inline Engine::Sprite sprStatusBar;

    static inline Engine::Sprite sprMenuButton;
    static inline Engine::Sprite sprPlayButton;
    static inline Engine::Sprite sprStopButton;
    static inline Engine::Sprite sprTrackButton;

    static inline const int aButtonUp = 0;
    static inline const int aButtonDown = 1;

    static inline Engine::Sprite sprTopBar;
    static inline Engine::Sprite sprTopBarPanel;

    static inline Engine::Sprite sprPlayheadBar;
    static inline Engine::Sprite sprPlayheadArrow;
    
    static inline Engine::Sprite sprEndSongMarker;

    static inline Engine::Sprite sprLoadDialog;
    static inline Engine::Sprite sprLoadSampleDialog;
    static inline Engine::Sprite sprSaveDialog;

    static inline Engine::Sprite sprCheckboxDrum;

    // -- Sounds --------------------------------------------
    static inline std::filesystem::path sndGrappleImpact;
    static inline std::filesystem::path sndGrappleHook;
    static inline std::filesystem::path sndGrappleShoot;
    static inline std::filesystem::path sndGrappleExtend;
    static inline std::filesystem::path sndGrappleReel;
    static inline std::filesystem::path sndLockOn;
    static inline std::filesystem::path sndHookEffect;

    static inline std::filesystem::path sndDeath;
    static inline std::filesystem::path sndDeathPlayer;

    static inline std::filesystem::path sndSlam;
};

#endif // _CONTENT_H