#include "Tritone.h"

#include <chrono>
#include <engine/DebugConsole.h>
#include <engine/Game.h>
#include <iostream>
#include <mutex>

#include "tritone_editor/components/NoteGrid.h"
#include "tritone_editor/components/TritoneEditor.h"

using namespace Engine;
using namespace TriTone;

// These should be redefined in a music singleton or something
namespace
{
    // Music
    constexpr float masterVolumeMax = 1.0f;
    constexpr float masterVolumeMin = 0.0f;

    // The number of frames it takes to ramp up to target event values
    constexpr int rampFrames = 150;

    // Device config constants
    const ma_format deviceFormat = ma_format_f32;
    constexpr int sampleRate = 44100;
    constexpr int bytesPerSample = sizeof(float);
    constexpr int deviceChannels = 2;

    // Pitching
    constexpr float baseFreq = 440.0f;
    const float twelveRoot2 = Math::pow(2.0f, 1.0f / 12.0f);
    constexpr int pitchOffset = -24 + 4 - 12;

    // Thread safe stuff
    std::mutex fileLoadMutex;
    std::mutex setLoadFilepathMutex;

    // Main audio data callback
    void dataCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
    {
        Playback *playback = static_cast<Playback *>(pDevice->pUserData);
        playback->update(pOutput, frameCount);
    }

    // TEST
    SampleData sineSampleData;
}

// -- SampleData --
void SampleData::setData(float *src, size_t size)
{
    freeData();

    m_dataSize = size;
    m_data = std::make_unique<float[]>(m_dataSize);
    memcpy(m_data.get(), src, size * bytesPerSample);
}

void Engine::TriTone::SampleData::setData(std::unique_ptr<float[]> &src, size_t size)
{
    freeData();

    m_dataSize = size;
    m_data = std::move(src);
}

void SampleData::freeData()
{
    if (m_data != nullptr)
    {
        m_data = nullptr;
    }
}

float SampleData::sizeBytes() const
{
    return m_dataSize * bytesPerSample;
}

float SampleData::get(float index) const
{
    return m_data[static_cast<int>(index)];
}

float SampleData::getSafe(float index) const
{
    while (index < 0)
    {
        index += m_dataSize;
    }
    while (index >= m_dataSize)
    {
        index -= m_dataSize;
    }
    return get(index);
}

// -- Voice --
Voice::Voice(SampleData *sampleData)
{
    setSampleData(sampleData);
}

void Voice::setSampleData(SampleData *sampleData)
{
    LB_ASSERT(sampleData != nullptr, "Sampledata cannot be nullptr.");

    m_sampleData = sampleData;
    m_dataIndex = 0.0f;
}

void Voice::nextFrame()
{
    incDataIndex();

    m_rampFramesLeft = Math::approach(m_rampFramesLeft, 0, 1);
    m_attackFramesLeft = Math::approach(m_attackFramesLeft, 0, 1);
    m_framesLeft = Math::approach(m_framesLeft, 0, 1);
}

void Voice::incDataIndex()
{
    // Increment index, wrap around
    float inc = Math::pow(twelveRoot2, m_pitch);

    m_dataIndex += inc;

    while (m_dataIndex > m_sampleData->size())
    {
        if (m_oneShot)
        {
            // Instantly kill voice
            m_framesLeft = 0;

            // So we don't do any release frames
            m_releaseActive = true;

            // On the off chance that we somehow evaluate again after this
            // make sure we don't read past the end of our data...
            m_dataIndex = 0;
            return;
        }

        m_dataIndex -= m_sampleData->size();
    }
}

float Voice::evalNone()
{
    float sample = m_sampleData->get(m_dataIndex);

    return sample;
}

float Voice::evalLagrange()
{
    float sample;

    float sampleA, sampleB, sampleC, sampleD;
    float c0, c1, c2, c3;
    float margin = m_dataIndex - 2;
    float subPos = m_dataIndex - static_cast<int>(m_dataIndex);

    sampleA = m_sampleData->getSafe(margin - 1);
    sampleB = m_sampleData->getSafe(margin);
    sampleC = m_sampleData->getSafe(margin + 1);
    sampleD = m_sampleData->getSafe(margin + 2);

    c0 = sampleB;
    c1 = sampleC - 1 / 3.0f * sampleA - 1 / 2.0f * sampleB - 1 / 6.0f * sampleD;
    c2 = 1 / 2.0f * (sampleA + sampleC) - sampleB;
    c3 = 1 / 6.0f * (sampleD - sampleA) + 1 / 2.0f * (sampleB - sampleC);

    sample = ((c3 * subPos + c2) * subPos + c1) * subPos + c0;

    return sample;
}

void Voice::setVelocity(float v)
{
    m_targetVelocity = v;

    if (m_velocity != m_targetVelocity)
    {
        m_rampFramesLeft = rampFrames;
    }
}

void Voice::setPan(float p)
{
    m_targetPan = p;

    if (m_pan != m_targetPan)
    {
        m_rampFramesLeft = rampFrames;
    }
}

float Voice::eval(bool rightChannel)
{
    if (!m_sampleData || m_muted)
    {
        return 0.0f;
    }

    float sample = 0.0f;

    switch (interp)
    {
    case Interp::None:
        sample = evalNone();
        break;

    case Interp::Lagrange:
        sample = evalLagrange();
        break;
    }

    float velocityValue;
    float panValue;
    if (m_rampFramesLeft)
    {
        float lerpFac = 1.0f - static_cast<float>(m_rampFramesLeft) / static_cast<float>(rampFrames);

        velocityValue = Math::lerp(m_velocity, m_targetVelocity, lerpFac);
        panValue = Math::lerp(m_pan, m_targetPan, lerpFac);
    }
    else
    {
        m_velocity = m_targetVelocity;
        m_pan = m_targetPan;

        velocityValue = m_velocity;
        panValue = m_pan;
    }

    // Apply velocity
    sample *= velocityValue;

    // Apply panning
    if (rightChannel)
    {
        float right = panValue < 0.5f ? (panValue / 0.5f) : 1.0f;
        sample *= right;
    }
    else
    {
        float left = panValue > 0.5f ? (0.5f - ((panValue - 0.5f) / 0.5f)) : 1.0f;
        sample *= Math::Max(left, 0.0f);
    }

    // Apply attack
    if (m_attackFramesLeft > 0)
    {
        float attackFac = 1.0f - (static_cast<float>(m_attackFramesLeft) / static_cast<float>(m_attackFrames));
        LB_ASSERT(attackFac <= 1.0f, "Attack factor should not be above 1.");
        sample *= attackFac;
    }

    // Apply release
    if (m_releaseActive)
    {
        // Ramp volume towards 0
        float releaseFac = (static_cast<float>(m_framesLeft) / static_cast<float>(m_releaseFrames));
        LB_ASSERT(releaseFac <= 1.0f, "Release factor should not be above 1.");
        sample *= releaseFac;
    }

    return sample;
}

void Voice::setPitch(int p)
{
    m_pitch = p + pitchOffset;
}

void Voice::changePitch(int amount)
{
    m_pitch += amount;
    printf("Pitch: %d\n", m_pitch);
}

const void Voice::debugPrintData() const
{
    printf("\b-- Data --\n");
    for (int i = 0; i < m_sampleData->size(); i++)
    {
        printf("%f\n", m_sampleData->get(i));
    }
}

// -- InstrumentTrack --
void InstrumentTrack::startNote(const NoteEvent &event, Playback *playback)
{
    /*
    - Find the first unused voice
        - Fill with info about this note. Make sure that velocity and panning are set
        - Kick off note playback
        - Add it to this track's list of voices
    */
    Voice *voice = playback->requestUnusedVoice();

    if (!voice)
    {
        // Couldn't start the note
        return;
    }

    // Setup voice
    voice->setPitch(event.pitch);
    voice->m_framesLeft = event.length * playback->m_framesPerBeat;
    voice->m_oneShot = m_oneShot;
    voice->m_muted = m_muted;

    // Set sample data
    voice->setSampleData(m_sampleData);

    // Push into our active voices
    m_voices.push_back(voice);
}

void InstrumentTrack::setVelocity(const Event &event)
{
    for (Voice *voice : m_voices)
    {
        voice->setVelocity(event.value);
    }
}

void InstrumentTrack::setPan(const Event &event)
{
    for (Voice *voice : m_voices)
    {
        voice->setPan(event.value);
    }
}

void InstrumentTrack::triggerEvents(const uint32_t position, Playback *playback)
{
    // -- Look for events at this position and apply them --
    if (m_noteEvents.count(position))
    {
        for (auto &note : m_noteEvents.at(position))
        {
            startNote(note, playback);
        }
    }

    // Get events at this song position
    if (m_velocityEvents.count(position))
    {
        setVelocity(m_velocityEvents.at(position));
    }

    if (m_panEvents.count(position))
    {
        setPan(m_panEvents.at(position));
    }
}

void InstrumentTrack::removeUnusedVoices()
{
    int len = static_cast<int>(m_voices.size());
    for (int j = len - 1; j >= 0; j--)
    {
        if (m_voices.at(j)->m_framesLeft <= 0)
        {
            // Remove voice from active
            m_voices.erase(m_voices.begin() + j);
        }
    }
}

void Engine::TriTone::InstrumentTrack::setMuted(bool muted)
{
    if (m_muted == muted)
    {
        return;
    }
    m_muted = muted;

    // Set all our voices to use the same muted value
    for (auto &voice : m_voices)
    {
        voice->m_muted = m_muted;
    }
}

void InstrumentTrack::clearEvents()
{
    m_noteEvents.clear();
    m_velocityEvents.clear();
    m_panEvents.clear();
}

// -- Playback --
void Playback::init(Game *game)
{
    // TEST: Allocate test sine wave
    {
        int dataSize = 100;
        float *data = new float[dataSize];
        for (int i = 0; i < dataSize; i++)
        {
            float fac = i / static_cast<float>(dataSize);
            fac *= Math::pi * 2.0f;

            data[i] = Math::sin(fac) * 0.4f;
        }
        sineSampleData.setData(data, dataSize);
        delete[] data;
    }

    // Get filepaths
    m_game = game;
    m_bgmPath = m_game->getDataPath() / "bgm/";
    m_samplePath = m_bgmPath / "sample/";

    // Setup device
    m_device = ma_device();

    m_deviceConfig = ma_device_config();
    m_deviceConfig = ma_device_config_init(ma_device_type_playback);
    m_deviceConfig.playback.format = deviceFormat;
    m_deviceConfig.playback.channels = deviceChannels;
    m_deviceConfig.sampleRate = sampleRate;
    // m_deviceConfig.periodSizeInFrames = bufferSize; // Can't directly set buffer size. Only a hint
    m_deviceConfig.dataCallback = dataCallback;
    m_deviceConfig.pUserData = this;

    if (ma_device_init(NULL, &m_deviceConfig, &m_device) != MA_SUCCESS)
    {
        printf("Error: Failed to initialize miniaudio device\n");
        return;
    }

    // Allocate buffer
    m_bufferSizeElements = m_device.playback.internalPeriodSizeInFrames * deviceChannels;
    m_bufferSizeBytes = bytesPerSample * m_bufferSizeElements;
    m_buffer = std::make_unique<float[]>(m_bufferSizeElements);
    memset(m_buffer.get(), 0, m_bufferSizeElements * bytesPerSample);

    // Fill m_unusedVoices
    for (int i = 0; i < m_voiceCount; i++)
    {
        m_inactiveVoices.push(&m_voices[i]);
    }

    // Start audio stream
    if (ma_device_start(&m_device) != MA_SUCCESS)
    {
        printf("Error: Failed to start device playback.\n");
    }
}

void Playback::update(void *pOutput, ma_uint32 frameCount)
{
    checkForNoteRequest();
    checkForPlaystateChange();
    checkForSongChange();
    generateSamples(pOutput, frameCount);
}

void Playback::free()
{
    ma_device_stop(&m_device);
    ma_device_uninit(&m_device);
}

void Playback::load(const std::filesystem::path &path, bool playOnLoad)
{
    const std::lock_guard<std::mutex> lock(setLoadFilepathMutex);

    if (path == m_fileLoaded)
    {
        printf("returned\n");
        m_playOnLoad.store(playOnLoad);
        return;
    }
    m_fileLoaded = path;

    // Load data into editor structs, then load into playback
    // I would do this much differently, given the time

    TritoneEditor tritone;
    NoteGrid noteGrid;

    // Allocate tracks in noteGrid
    for (int i = 0; i < TritoneEditCommon::trackCount; i++)
    {
        TritoneEditCommon::addTrack(tritone, noteGrid);
    }
    TritoneEditCommon::loadTritoneEditor(tritone, noteGrid, m_bgmPath / path);
    loadFromEditor(tritone, noteGrid);

    // Make sure this comes AFTER everything... otherwise we can get data race
    m_playOnLoad.store(playOnLoad);
}

void Playback::loadAndPlay(const std::filesystem::path &path)
{
    load(path, true);
}

void Playback::loadFromEditor(TritoneEditor &editor, NoteGrid &noteGrid)
{
    // Try and prevent any song data structures from being accessed from audio thread
    const std::lock_guard<std::mutex> lock(fileLoadMutex);

    // LB_ASSERT(!isPlaying(), "Cannot load song data while playing.");
    auto timeStart = std::chrono::high_resolution_clock::now();

    // Load song data
    m_song.m_bpm = editor.bpm;
    m_song.m_endPosition = editor.endSongMarker;
    m_fileLoaded = editor.saveFilepath;

    // m_song.m_repeatFrom = editor.

    // Calculate samples per beat using bpm
    {
        double beatsPerSecond = static_cast<double>(m_song.m_bpm) / 60.0;
        m_samplesPerBeat = sampleRate / static_cast<double>(beatsPerSecond);
        m_framesPerBeat = m_samplesPerBeat / 4.0;
    }

    // Load from editor data structures
    for (int i = 0; i < Song::trackCount; i++)
    {
        // Get playback data structures
        InstrumentTrack &playbackTrack = m_song.m_tracks.at(i);
        auto &playbackNoteEvents = playbackTrack.m_noteEvents;

        const int numEventTypes = 2;
        std::array<InstrumentTrack::EventMap *, numEventTypes> playbackEventMaps = {
            &playbackTrack.m_velocityEvents,
            &playbackTrack.m_panEvents,
        };

        // Get editor data structures
        NoteGrid::TrackData &editorTrack = noteGrid.tracks.at(i);
        std::array<NoteGrid::EventMap *, numEventTypes> editorEventMaps = {
            &editorTrack.velocityEvents,
            &editorTrack.panEvents,
        };

        // Clear playback track events
        playbackTrack.clearEvents();

        // Transfer note events
        for (const auto &pair : editorTrack.columnData)
        {
            uint32_t position = pair.first;
            auto &editorNoteList = pair.second;

            // Insert new column into playback notes
            playbackNoteEvents.insert({position, {}});
            auto &playbackNoteList = playbackNoteEvents.at(position);

            // Copy editor notes into playback noteMap
            for (auto &editorNote : editorNoteList)
            {
                // Create new playback note
                playbackNoteList.emplace_back();

                NoteEvent &playbackNote = playbackNoteList.back();
                playbackNote.length = editorNote.length;
                playbackNote.pitch = editorNote.pitch;
            }
        }

        // Transfer other events
        for (int i = 0; i < numEventTypes; i++)
        {
            auto *editorEventMap = editorEventMaps.at(i);
            auto *playbackEventMap = playbackEventMaps.at(i);

            for (auto &pair : *editorEventMap)
            {
                uint32_t position = pair.first;
                const auto &editorEvent = pair.second;

                Event playbackEvent;
                playbackEvent.value = editorEvent.value;
                playbackEventMap->insert({position, playbackEvent});
            }
        }

        // Set track info
        playbackTrack.m_oneShot = editorTrack.oneshot;
        playbackTrack.setMuted(editorTrack.muted);

        // Load sample data
        if (editorTrack.samplePath != "")
        {
            playbackTrack.m_sampleData = loadOrFetchSampleData(m_samplePath / editorTrack.samplePath);
        }
        else
        {
            // Just use sine wave
            playbackTrack.m_sampleData = &sineSampleData;
        }
    }

    // Load sample data into instrument tracks
    // TODO: Put this into another function that only gets called when you change an instrument track

    std::chrono::duration<float, std::milli> duration = std::chrono::high_resolution_clock::now() - timeStart;
    std::cout << "Parse completed in " << duration.count() << " ms\n";
}

void Playback::play()
{
    // TODO: Seek to current position
    m_playing.store(true);
}

void Playback::pause()
{
    m_playing.store(false);
}

void Playback::requestPlayNote(uint8_t pitch, uint16_t length, uint8_t track)
{
    m_playNotePitch.store(pitch);
    m_playNoteLength.store(length);
    m_playNoteTrack.store(track);
}

void Playback::requestSetPlayheadStart(uint32_t startPos)
{
    m_playheadStartPos.store(startPos);
}

void Playback::seek(uint32_t seekPos)
{
    if (m_song.m_endPosition > 0 && seekPos > (m_song.m_endPosition - 1))
    {
        // If we reached the end, wrap around to the beginning
        m_playhead.store(0);
        return;
    }
    m_playhead.store(seekPos);
}

SampleData *Engine::TriTone::Playback::loadOrFetchSampleData(const std::filesystem::path &path)
{
    if (m_samples.count(path))
    {
        return &m_samples.at(path);
    }

    std::unique_ptr<float[]> buffer;
    ma_uint64 totalFrames = 0;
    ma_uint64 framesRead = 0;

    ma_decoder decoder;
    ma_decoder_config config;
    ma_result result;

    // Insert a new entry into the samples map
    m_samples.insert({path, SampleData()});
    SampleData &newSample = m_samples.at(path);

    // -- Load the file with miniaudio --
    config = ma_decoder_config_init(deviceFormat, deviceChannels, sampleRate);
    config.encodingFormat = ma_encoding_format_wav;

    result = ma_decoder_init_file(path.string().c_str(), &config, &decoder);

    if (result != MA_SUCCESS)
    {
        // goto :( but should be alright in this situation
        goto sampleFailedInit;
    }

    // -- Read the file --

    // Get length of file
    result = ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames);
    if (result != MA_SUCCESS || totalFrames == 0)
    {
        goto sampleReadFailed;
    }

    // Allocate buffer of that size
    buffer = std::make_unique<float[]>(totalFrames * deviceChannels);

    // Read file into buffer
    result = ma_decoder_read_pcm_frames(&decoder, buffer.get(), totalFrames, &framesRead);
    if (result != MA_SUCCESS || framesRead < totalFrames)
    {
        goto sampleReadFailed;
    }

    // Copy buffer into sample data
    newSample.setData(buffer, totalFrames * deviceChannels);
    buffer = nullptr;

    ma_decoder_uninit(&decoder);
    return &newSample;

    // Exit with errors
sampleReadFailed:
    ma_decoder_uninit(&decoder);

sampleFailedInit:

    // Undo our insert from earlier...
    m_samples.erase(path);

    printf("Could not read sample '%s'. Falling back to default sin...\n", path.string().c_str());
    return &sineSampleData;
}

Voice *Playback::requestUnusedVoice()
{
    if (m_inactiveVoices.size() <= 0)
    {
        // sorry bro
        return nullptr;
    }

    // Pop a voice off inactive voices
    Voice *voice = m_inactiveVoices.front();
    m_inactiveVoices.pop();

    // Push into activeVoices
    m_activeVoices.push_back(voice);

    // Initialize voice
    *voice = Voice();

    return voice;
}

void Playback::triggerTrackEvents()
{
    for (auto &track : m_song.m_tracks)
    {
        track.triggerEvents(playheadPos(), this);
    }
}

void Playback::killAllVoices()
{
    int len = static_cast<int>(m_activeVoices.size());

    for (int j = len - 1; j >= 0; j--)
    {
        Voice *voice = m_activeVoices.at(j);
        voice->m_framesLeft = 0;
    }
}

void Playback::startPlayingInternal()
{
    // Called by audio thread when we just started playing
    m_playingInternal = true;
    seek(m_playheadStartPos.load());

    m_beatProgress = 0;

    triggerTrackEvents();
}

void Playback::stopPlayingInternal()
{
    // Called by audio thread when we just stopped playing
    m_playingInternal = false;

    killAllVoices();
}

void Playback::checkForNoteRequest()
{
    int pitch = m_playNotePitch.load();
    int length = m_playNoteLength.load();
    int track = m_playNoteTrack.load();

    if (pitch == -1 ||
        length == -1 ||
        track == -1)
    {
        return;
    }

    NoteEvent note;
    note.pitch = pitch;
    note.length = length;
    m_song.m_tracks.at(track).startNote(note, this);

    m_playNotePitch.store(-1);
    m_playNoteLength.store(-1);
    m_playNoteTrack.store(-1);
}

void Playback::generateSamples(void *pOutput, ma_uint32 frameCount)
{
    int bpf = ma_get_bytes_per_frame(deviceFormat, deviceChannels);
    int bps = ma_get_bytes_per_sample(deviceFormat);

    double framesLeft = frameCount;

    // Pointer inside our internal buffer
    float *p = (float *)m_buffer.get();

    // Zero output buffer
    memset(pOutput, 0, frameCount * bpf);

    // Zero our internal buffer
    memset((char *)m_buffer.get(), 0, frameCount * bpf);

    while (framesLeft > 0)
    {
        if (m_beatProgress >= m_framesPerBeat && m_framesPerBeat > 0)
        {
            while (m_beatProgress >= m_framesPerBeat)
            {
                m_beatProgress -= m_framesPerBeat;
            }

            // Move to the next beat
            seek(playheadPos() + 1);

            triggerTrackEvents();
        }

        // Calculate the number of frames we need to do
        double framesToDo = Math::Min(m_framesPerBeat - m_beatProgress, framesLeft);
        framesLeft -= framesToDo;

        // Prevent deadlock in a situation where we haven't loaded a song yet
        if (framesToDo <= 0 && framesLeft > 0)
        {
            framesLeft -= framesLeft;
        }

        // Prevent deadlock where framesToDo becomes really small
        if (m_framesPerBeat - m_beatProgress < Math::epsilon)
        {
            m_beatProgress = m_framesPerBeat;
            continue;
        }

        if (m_playingInternal)
        {
            m_beatProgress += framesToDo;
        }

        // Generate samples
        for (int i = 0; i < framesToDo; i++)
        {
            int len = static_cast<int>(m_activeVoices.size());
            for (int j = len - 1; j >= 0; j--)
            {
                Voice *voice = m_activeVoices.at(j);

                // Create left sample, then right sample
                for (int k = 0; k < deviceChannels; k++)
                {
                    *p += voice->eval(static_cast<bool>(k));
                    p++;
                }

                // Return to beginning of frame
                p -= deviceChannels;

                // Increment data pointer inside of voice, decrease frames left
                voice->nextFrame();
                if (voice->m_framesLeft <= 0)
                {
                    if (!voice->m_releaseActive && voice->m_releaseFrames > 0)
                    {
                        // Activate release if we haven't done it yet
                        voice->m_releaseActive = true;
                        voice->m_framesLeft = voice->m_releaseFrames;
                    }
                    else
                    {
                        // Add to unused voices
                        m_inactiveVoices.push(voice);
                        // Remove voice from active
                        m_activeVoices.erase(m_activeVoices.begin() + j);
                    }
                }
            }

            // Move forward in stream
            p += deviceChannels;
        }

        for (auto &track : m_song.m_tracks)
        {
            track.removeUnusedVoices();
        }
    }

    // Do final mixing
    p = (float *)m_buffer.get();
    float masterVol = m_masterVolume.load();
    for (int i = 0; i < frameCount * deviceChannels; i++)
    {
        // Scale to master volume
        *p *= masterVol;

        // Clip at 1.0/-1.0
        *p = Math::clamp(*p, -masterVolumeMax, masterVolumeMax);

        p++;
    }

    // Copy to output buffer
    // memcpy(pOutput, m_buffer.get(), m_bufferSizeBytes);
    memcpy(pOutput, m_buffer.get(), frameCount * bpf);
}

void Playback::checkForSongChange()
{
    if (m_playOnLoad.load())
    {
        // Stop sound from previous song
        pause();
        stopPlayingInternal();

        // Seek to the beginning of the file
        seek(0);
        m_beatProgress = 0;

        // Start playing new song
        play();
        startPlayingInternal();

        m_playOnLoad.store(false);
    }
}

void Playback::checkForPlaystateChange()
{
    if (m_playingInternal && !isPlaying())
    {
        // Stopped playing
        stopPlayingInternal();
    }
    else if (!m_playingInternal && isPlaying())
    {
        // Started playing
        startPlayingInternal();
    }
}

std::filesystem::path Engine::TriTone::Playback::getCurrentSongFilepath() const
{
    const std::lock_guard<std::mutex> lock(setLoadFilepathMutex);
    return m_fileLoaded;
}

float Playback::getMasterVolume()
{
    return m_masterVolume.load();
}

void Playback::setMasterVolume(float newVol)
{
    m_masterVolume.store(Math::clamp(newVol, masterVolumeMin, masterVolumeMax));
}
