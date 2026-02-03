#include "components/TritoneEditor.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include <engine/DebugConsole.h>
#include <engine/Spatial.h>

using namespace Engine;

/*
-- .tri file spec --

4 char "TRTN" magic header
u16 - version number
u16 - tempo (BPM)
u32 - repeat position
u32 - end song position
u8 	- number of event types (velocity, pan)
u16 - number of tracks

PER-TRACK (instrument data):
--  Null-terminated string - sample data filename
--  u8 	- oneshot (0 or 1)
--  u32 - sample loop start
--  u32 - sample loop end
--  u32 - note event count
--  u32 - velocity event count
--  u32 - pan event count

PER-TRACK (event data):
--	PER-noteCount
--	--	u32 - position
--	--	u8 	- pitch
--	--	u16 - length
--	PER-velocityCount
--	--	u32 	- position
--	--	float32 - value
--	PER-panCount
--	--	u32 	- position
--	--	float32 - value

*/

namespace
{
	std::string magicHeader = "TRTN";
	constexpr uint16_t version = 0;
	constexpr uint16_t trackCount = 16;
	constexpr uint8_t numEventTypes = 2;
	std::string defaultSampleData = "DUMMY";

	// -- File IO funcs --
	void writeInt(std::ofstream &f, int value, int size)
	{
		f.write(reinterpret_cast<const char *>(&value), size);
	}

	template <typename T, unsigned length = sizeof(T)>
	T readBytes(std::ifstream &f)
	{
		T temp;
		f.read((char *)&temp, length);
		return temp;
	}

	template <unsigned length>
	std::string readString(std::istream &stream)
	{
		char buf[length + 1] = {0};
		stream.read(buf, length);
		return std::string(buf);
	}

	// -- Helpers --
	uint32_t countNotes(NoteGrid::NoteMap &columnData)
	{
		uint32_t noteCount = 0;
		for (const auto &pair : columnData)
		{
			auto &noteList = pair.second;
			noteCount += noteList.size();
		}
		return noteCount;
	}

	uint32_t countEvents(NoteGrid::EventMap &eventMap)
	{
		return static_cast<uint32_t>(eventMap.size());
	}

	void writeEvents(std::ofstream &f, NoteGrid::EventMap &eventMap)
	{
		for (auto &pair : eventMap)
		{
			uint32_t position = pair.first;
			const auto &event = pair.second;

			writeInt(f, position, sizeof(uint32_t));
			f.write((char *)&event.value, sizeof(float));
		}
	}

	void readEvents(std::ifstream &f, uint32_t numEvents, NoteGrid::EventMap &eventMap)
	{
		for (int i = 0; i < numEvents; i++)
		{
			NoteGrid::EventMarker event;

			event.position = readBytes<uint32_t>(f);
			event.value = readBytes<float>(f);

			eventMap.insert_or_assign(event.position, event);
		}
	}
}

namespace TritoneEditCommon
{
	void saveTritone(TritoneEditor &tritone, NoteGrid &noteGrid, const std::filesystem::path &outPath)
	{
		auto timeStart = std::chrono::high_resolution_clock::now();

		uint32_t a_noteCount[trackCount];
		uint32_t a_velocityCount[trackCount];
		uint32_t a_panCount[trackCount];

		std::ofstream f;
		f.open(outPath, std::ios::binary);

		LB_ASSERT(f.is_open(), "Could not write to file.");

		// Write magic header
		f.write(magicHeader.c_str(), 4);

		// Version number
		writeInt(f, version, sizeof(uint16_t));

		// Tempo
		writeInt(f, tritone.bpm, sizeof(uint16_t));

		// Repeat position (unused)
		writeInt(f, 0, sizeof(uint32_t));

		// End song position
		writeInt(f, tritone.endSongMarker, sizeof(uint32_t));

		// Number of event types
		writeInt(f, numEventTypes, sizeof(uint8_t));

		// Track count
		writeInt(f, trackCount, sizeof(uint16_t));

		// -- Write track info chunk --
		for (int i = 0; i < trackCount; i++)
		{
			NoteGrid::TrackData &editorTrack = noteGrid.tracks.at(i);
			std::string sampleName = editorTrack.samplePath.filename().string();

			// Sample data path
			if (editorTrack.samplePath.string().size() > 0)
			{
				f.write(sampleName.c_str(), sampleName.size() + 1);
			}
			else
			{
				f.write(defaultSampleData.c_str(), defaultSampleData.size() + 1);
			}

			// Oneshot?
			writeInt(f, static_cast<uint8_t>(editorTrack.oneshot), sizeof(uint8_t));

			// Sample loop start
			writeInt(f, 0, sizeof(uint32_t));

			// Sample loop end
			writeInt(f, 0, sizeof(uint32_t));

			// Count events
			{
				// Count notes
				a_noteCount[i] = countNotes(editorTrack.columnData);
				writeInt(f, a_noteCount[i], sizeof(uint32_t));

				// Count velocity events
				a_velocityCount[i] = countEvents(editorTrack.velocityEvents);
				writeInt(f, a_velocityCount[i], sizeof(uint32_t));

				// Count pan events
				a_panCount[i] = countEvents(editorTrack.panEvents);
				writeInt(f, a_panCount[i], sizeof(uint32_t));
			}
		}

		// -- Write track event data chunk --
		for (int i = 0; i < trackCount; i++)
		{
			NoteGrid::TrackData &editorTrack = noteGrid.tracks.at(i);

			// Write note data
			for (const auto &pair : editorTrack.columnData)
			{
				uint32_t position = pair.first;
				auto &noteList = pair.second;

				for (auto &note : noteList)
				{
					// Write note position
					writeInt(f, position, sizeof(uint32_t));

					// Write note pitch
					writeInt(f, note.pitch, sizeof(uint8_t));

					// Write note length
					writeInt(f, note.length, sizeof(uint16_t));
				}
			}

			writeEvents(f, editorTrack.velocityEvents);
			writeEvents(f, editorTrack.panEvents);
		}

		f.close();

		std::chrono::duration<float, std::milli> duration = std::chrono::high_resolution_clock::now() - timeStart;
		std::cout << "Wrote to " << outPath.filename() << " in " << duration.count() << " ms\n";
	}

	// Load tritone into editor
	bool loadTritoneEditor(TritoneEditor &tritone, NoteGrid &noteGrid, const std::filesystem::path &inPath)
	{
		uint32_t a_noteCount[trackCount];
		uint32_t a_velocityCount[trackCount];
		uint32_t a_panCount[trackCount];
		std::string a_samplePaths[trackCount];
		bool a_oneshot[trackCount];

		auto timeStart = std::chrono::high_resolution_clock::now();

		std::ifstream f;
		f.open(inPath, std::ios::binary);

		LB_ASSERT(f.is_open(), "Could not read file.");

		// Read magic header
		std::string magic = readString<4>(f);
		if (magic != magicHeader)
		{
			return true;
		}

		// Clear editor tracks
		initializeEditor(tritone, noteGrid);

		// Version number
		uint16_t version = readBytes<uint16_t>(f);

		// Tempo
		tritone.bpm = readBytes<uint16_t>(f);

		// Repeat position (unused)
		readBytes<uint32_t>(f);

		// End song position
		tritone.endSongMarker = readBytes<uint32_t>(f);

		// Number of event types
		readBytes<uint8_t>(f);

		// Track count
		readBytes<uint16_t>(f);

		// -- Read track info chunk --
		for (int i = 0; i < trackCount; i++)
		{
			// Sample data path
			std::string samplePath;
			std::getline(f, samplePath, '\0');

			if (samplePath != defaultSampleData)
			{
				a_samplePaths[i] = samplePath;
			}
			else
			{
				a_samplePaths[i] = "";
			}

			// Oneshot?
			a_oneshot[i] = readBytes<uint8_t>(f);

			// Sample loop start
			readBytes<uint32_t>(f);

			// Sample loop end
			readBytes<uint32_t>(f);

			// Note count
			a_noteCount[i] = readBytes<uint32_t>(f);

			// Count velocity events
			a_velocityCount[i] = readBytes<uint32_t>(f);

			// Count pan events
			a_panCount[i] = readBytes<uint32_t>(f);
		}

		for (int i = 0; i < trackCount; i++)
		{
			NoteGrid::TrackData &editorTrack = noteGrid.tracks.at(i);
			auto &columnData = editorTrack.columnData;
			auto &velocityEvents = editorTrack.velocityEvents;
			auto &panEvents = editorTrack.panEvents;

			// Set sample path
			editorTrack.samplePath = a_samplePaths[i];
			// Set oneshot
			editorTrack.oneshot = a_oneshot[i];

			// Read note data
			for (int j = 0; j < a_noteCount[i]; j++)
			{
				// NoteGrid::NoteList noteList;
				uint32_t position;
				NoteGrid::Note note;

				position = readBytes<uint32_t>(f);
				note.pitch = readBytes<uint8_t>(f);
				note.length = readBytes<uint16_t>(f);

				// Setup pos for editor
				note.pos = Vec2i(position, note.pitch);

				addNoteData(tritone, noteGrid, note, i);
			}

			readEvents(f, a_velocityCount[i], velocityEvents);
			readEvents(f, a_panCount[i], panEvents);
		}

		f.close();
		std::chrono::duration<float, std::milli> duration = std::chrono::high_resolution_clock::now() - timeStart;
		std::cout << "Read " << inPath.filename() << " in " << duration.count() << " ms\n";

		return false;
	}
}
