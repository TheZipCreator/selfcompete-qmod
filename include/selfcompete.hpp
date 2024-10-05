#pragma once

#include <cstdio>
#include <vector>

#include "HMUI/CurvedTextMeshPro.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"

#include "main.hpp"

#define SELFCOMPETE_FORMAT_VERSION 0

#define SELFCOMPETE_BEATLEADER_DIR "/beatleader"

namespace selfcompete {
	// keeps a score
	class ScoreKeeper {
		public:
		FILE *fp; // pointer to the file this ScoreKeeper is using
		int score; // current score

		virtual void update(float time, int score); // updates the score keeper
		~ScoreKeeper();
	};
	class ReadingScoreKeeper : public ScoreKeeper {
		float timeRead; // time last score was read
		int scoreRead; // last score read
		public:

		void update(float time, int score) override;


		ReadingScoreKeeper(FILE *fp);
	};
	class RecordingScoreKeeper : public ScoreKeeper {
		public:
		void update(float time, int score) override;
		
		RecordingScoreKeeper(FILE *fp);
	};
	// struct that stores state
	struct State {
		bool activated; // whether selfcompete is currently activated or not
		SafePtrUnity<HMUI::CurvedTextMeshPro> rank_text; // text that states the rank
		std::vector<std::unique_ptr<ScoreKeeper>> score_keepers; // vector of score keepers
		FILE *recorder_fp; // pointer to the recorder file pointer
		std::string recorder_file; // path to recorder file
	};
	extern State state;

	// registers hooks
	void register_selfcompete_hooks(void);
	
	// gets a level directory
	std::string get_level_dir(GlobalNamespace::BeatmapKey key);
	// gets a level directory
	std::string get_level_dir(std::string id, std::string characteristic, std::string difficulty);
	// activates selfcompete
	void activate(GlobalNamespace::BeatmapKey key);
	// deactivates selfcompete
	void deactivate(bool save_recording = true);
	// updates selfcompete
	void update(float time, int score);
}
