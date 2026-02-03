#ifndef _TRITONE_H
#define _TRITONE_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <array>
#include <queue>
#include <filesystem>
#include <atomic>

#include <vendor/miniaudio/miniaudio.h>

// Playback engine for my "TriTone" music format

class TritoneEditor;
class NoteGrid;

namespace Engine
{
    class Game;
    
    namespace TriTone
    {
        struct NoteEvent;
        struct VelocityEvent;
        struct PanEvent;

        class SampleData;
        class Voice;
        class InstrumentTrack;
        class Song;

        struct NoteEvent
        {
            // Position in song
            // uint32_t pos = 0;
            // (0, 128?)
            int8_t pitch = 0;
            // Length of the note
            uint16_t length = 1;
        };

        struct Event
        {
            float value;
        };

        // Contains raw audio data that can be played by a voice
        class SampleData
        {
        private:
            // Raw audio data (-1.0 to 1.0 f32)
            std::unique_ptr<float[]> m_data = nullptr;
            // Size of the data (in samples)
            int m_dataSize = 0;

            // TODO:
            // How much to detune the sample by
            // float detune = 0.0f;

        public:
            SampleData() = default;

            // No copy/move
            // SampleData(SampleData &&) = delete;
            // SampleData(const SampleData &) = delete;
            // SampleData &operator=(SampleData &&) = delete;
            // SampleData &operator=(const SampleData &) = delete;

            // Memcpy src into data (size is in array elements, not bytes)
            void setData(float *src, size_t size);
            void setData(std::unique_ptr<float[]> &src, size_t size);
            // Free memory
            void freeData();

            inline float size() const { return m_dataSize; }
            float sizeBytes() const;

            // Index right into the data array without bounds checking. Will cast down to an int
            float get(float index) const;
            // Index into data. Wrap around if out of bounds
            float getSafe(float index) const;
        };

        // Plays SampleData
        class Voice
        {
        public:
            enum class Interp
            {
                None,
                Lagrange,
                Count,
            } interp = Interp::Lagrange;

        private:
            // Pointer to sample date
            SampleData *m_sampleData = nullptr;
            float m_dataIndex = 0.0f;

            int m_pitch = 0;

            // Volume ramping
            float m_targetVelocity = 0.85f;
            float m_targetPan = 0.5f;
            int m_rampFramesLeft = 0;
            
            float m_velocity = m_targetVelocity;
            float m_pan = m_targetPan;

            float evalNone();
            float evalLagrange();

        public:
            // Do we play our sample only once?
            bool m_oneShot = false;

            // Are we muted?
            bool m_muted = false;
        
            // Frames left to do before we're done
            int m_framesLeft = 0;
            
            // Frames of attack
            int m_attackFrames = 100;
            int m_attackFramesLeft = m_attackFrames;
            
            // Are we currently in the release part of the envelope?
            float m_releaseActive = false;
            // Frames of release
            int m_releaseFrames = 150;

            Voice() = default;
            Voice(SampleData *sampleData);

            void setSampleData(SampleData *sampleData);

            // Output a single sample
            float eval(bool rightChannel);

            // Increment to the next frame
            void nextFrame();
            
            // Increment data index. Wrap around if out of bounds
            void incDataIndex();

            inline float getVelocity() const { return m_velocity; }
            void setVelocity(float v);
            inline float getPan() const { return m_pan; }
            void setPan(float p);

            // Set the note pitch
            void setPitch(int p);
            // Change the pitch by a relative amount
            void changePitch(int amount = 1);

            // -- Debug --
            const void debugPrintData() const;
        };

        // Directs voices to play sample data
        class InstrumentTrack
        {
            friend class Playback;

        public:
            using NoteVector = std::vector<NoteEvent>;
            using NoteEventMap = std::unordered_map<uint32_t, NoteVector>; // TODO: Will have to map to a list of noteevents instead
            using EventMap = std::unordered_map<uint32_t, Event>;

        protected:
            NoteEventMap m_noteEvents;
            EventMap m_velocityEvents;
            EventMap m_panEvents;

            SampleData *m_sampleData = nullptr;

            // Loop sample data?
            bool m_oneShot = false;

            // Should we play?
            bool m_muted = false;

            // List of the voices that this track is using
            std::vector<Voice *> m_voices;

            void startNote(const NoteEvent &event, class Playback *playback);
            void setVelocity(const Event &event);
            void setPan(const Event &event);

        public:
            // Trigger all events on this column
            void triggerEvents(const uint32_t position, class Playback *playback);
            void removeUnusedVoices();
            
            void setMuted(bool muted);

            // Delete all events from this track
            void clearEvents();
        };

        class Song
        {
        public:
            // The position of the end of the song
            uint32_t m_endPosition = 0;
            // Position to repeat from in cells (UNUSED)
            // uint32_t m_repeatFrom = 0;

            // Tempo of the song
            uint16_t m_bpm = 140;

            // Track data
            static const int trackCount = 16;
            std::array<InstrumentTrack, trackCount> m_tracks;

            // Sample data
            std::vector<std::unique_ptr<SampleData>> m_sampleDataList;

            inline void clearTracks()
            {
                for (int i = 0; i < trackCount; i++)
                {
                    m_tracks[i] = InstrumentTrack();
                }
            }
        };

        // The main Tritone performance engine
        class Playback
        {
            friend class InstrumentTrack;

        private:
            // Pointer to game class
            class Game *m_game = nullptr;
        
            // Number of samples per beat
            double m_samplesPerBeat = 0;
            double m_framesPerBeat = 0;
            // Number of frames we are into this beat
            double m_beatProgress = 0;

            static const int m_voiceCount = 128;
            Voice m_voices[m_voiceCount];

            // List of pointers to voices that aren't in use
            std::queue<Voice *> m_inactiveVoices{};
            std::vector<Voice *> m_activeVoices{};

            // Miniaudio stuff
            ma_device m_device;
            ma_device_config m_deviceConfig;
            
            uint32_t m_bufferSizeElements = 0;
            uint32_t m_bufferSizeBytes = 0;
            std::unique_ptr<float[]> m_buffer;
            
            // The song we currently have loaded
            std::filesystem::path m_fileLoaded;

            // Grab an unused voice if possible. If none available, returns nullptr
            Voice *requestUnusedVoice();

            // Trigger track events that they're sitting on
            void triggerTrackEvents();

            // Stop all voices immediately
            void killAllVoices();

            bool m_playingInternal = false;
            void startPlayingInternal();
            void stopPlayingInternal();

            // -- Threadsafe stuff --
            std::atomic<bool> m_playing = false;
            std::atomic<float> m_masterVolume = 0.25f;

            std::atomic<int> m_playNotePitch = -1;
            std::atomic<int> m_playNoteLength = -1;
            std::atomic<int> m_playNoteTrack = -1;

            // Position of the playhead in the song (in cells)
            std::atomic<uint32_t> m_playhead = 0;

            // Where the editor has requested we start from
            std::atomic<uint32_t> m_playheadStartPos = 0;

            // Should we start playing the song as soon as it's loaded?
            std::atomic<bool> m_playOnLoad = false;

            // Check if a note was requested to be played, then play it if so
            void checkForNoteRequest();
            // Did the main thread try to change if we're playing or not?
            void checkForPlaystateChange();
            // Did we just change songs?
            void checkForSongChange();
        public:
            // Song data
            Song m_song;

            std::unordered_map<std::filesystem::path, SampleData> m_samples;

            // Filepaths
            std::filesystem::path m_bgmPath = "";
            std::filesystem::path m_samplePath = "";
            
            Playback() = default;

            // Init audio engine
            void init(class Game *game);
            // Called from the audio thread
            void update(void *pOutput, ma_uint32 frameCount);
            // Free audio engine
            void free();

            // Seek to a part in the song
            void seek(uint32_t seekPos);
            
            // Load a sample if it hasn't already been loaded and return a pointer to the SampleData
            SampleData* loadOrFetchSampleData(const std::filesystem::path& path);

            // Generate samples and progress song
            // Don't call outside of engine code
            void generateSamples(void *pOutput, ma_uint32 frameCount);
            
            // -- Thread safe accessors --
            // Load song from a file
            void load(const std::filesystem::path &path, bool playOnLoad = false);
            
            // Load a song from a file and play
            void loadAndPlay(const std::filesystem::path &path);
            
            // Load song data from the editor
            void loadFromEditor(class TritoneEditor &editor, class NoteGrid &noteGrid);

            // -- Editor thread safe stuff --
            // Play the song
            void play();
            // Pause the song
            void pause();

            // Threadsafe way to play a note (NOTE: really fast requests can be dropped)
            void requestPlayNote(uint8_t pitch, uint16_t length, uint8_t track);

            // Request that we set the start position of the playhead
            void requestSetPlayheadStart(uint32_t startPos);
            
            bool isPlaying() { return m_playing.load(); }
            uint32_t playheadPos() { return m_playhead.load(); }

            // Get the filepath of the song we have loaded
            std::filesystem::path getCurrentSongFilepath() const;

            float getMasterVolume();
            void setMasterVolume(float newVol);
        };
    }
}

#endif // _TRITONE_H