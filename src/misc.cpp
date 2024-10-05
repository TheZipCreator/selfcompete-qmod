#include <fstream>
#include <cstdio>

#include "GlobalNamespace/ScoreModel.hpp"
#include "GlobalNamespace/NoteData.hpp"

#include "misc.hpp"
#include "main.hpp"
#include "selfcompete.hpp"

#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/utils.h"

#include "bsor/bsor.hpp"

namespace selfcompete {
	std::string difficulty_to_string(GlobalNamespace::BeatmapDifficulty dif) {
		if(dif == GlobalNamespace::BeatmapDifficulty::Easy)
			return "Easy";
		if(dif == GlobalNamespace::BeatmapDifficulty::Normal)
			return "Normal";
		if(dif == GlobalNamespace::BeatmapDifficulty::Hard)
			return "Hard";
		if(dif == GlobalNamespace::BeatmapDifficulty::Expert)
			return "Expert";
		if(dif == GlobalNamespace::BeatmapDifficulty::ExpertPlus)
			return "ExpertPlus";
		PaperLogger.warn("difficulty_to_string() called on invalid difficulty, with value {}.", static_cast<int32_t>(dif));
		return "Unknown";
	}
	std::string generate_filename(void) {
		std::string name(64, ' ');
		std::string acceptable_chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		for(int i = 0; i < 64; i++) {
			name[i] = acceptable_chars[std::rand()%acceptable_chars.size()];
		}
		return name;
	}
	std::string get_tmp_dir(void) {
		std::string dir = getDataDir(mod_info)+"/tmp";
		if(!direxists(dir))
			mkpath(dir);
		return dir;
	}
	bool convert_replay(std::string in_file, std::string out_file) {
		// yes, I know I'm using both the C++ io and C io here.
		// do I care? no.
		std::ifstream stream(in_file);
		auto result = bsor::read_bsor(stream);
		if(auto bsor_ptr = std::get_if<std::shared_ptr<bsor::BSOR>>(&result)) {
			auto bsor = *bsor_ptr;
			FILE *fp = fopen(out_file.c_str(), "wb");
			if(fp == nullptr) {
				PaperLogger.warn("Could not open output file '{}'.", out_file);
				return false;
			}
			int version = SELFCOMPETE_FORMAT_VERSION;
			fwrite(&version, sizeof(int), 1, fp);
			#define WRITE(TIME) do { \
				fwrite(&TIME, sizeof(float), 1, fp); \
				fwrite(&score, sizeof(int), 1, fp); \
			} while(0)
			// this code is based heavily on https://github.com/Metalit/Replay/blob/master/src/Utils.cpp#L476

			// variables
			int multiplier = 1, progress = 0; // multiplier and progress towards next
			float last_wall = 0, wall_end = 0; // wall score info
			int score = 0; // current score
			float energy = 0.5; // energy
			std::vector<float> modifiers; // how modifiers affect score
			bool one_life = false, four_lives = false;
			for(std::string &mod : bsor->info.modifiers) {
				if(mod == "IF")
					one_life = true;
				else if(mod == "BE")
					four_lives = true;
				else if(mod == "NF")
					modifiers.push_back(-0.5);
				else if(mod == "NO")
					modifiers.push_back(-0.05);
				else if(mod == "NB")
					modifiers.push_back(-0.10);
				else if(mod == "SS")
					modifiers.push_back(-0.30);
				else if(mod == "DA")
					modifiers.push_back(0.07);
				else if(mod == "FS")
					modifiers.push_back(0.08);
				else if(mod == "SF")
					modifiers.push_back(0.22);
				else if(mod == "GN")
					modifiers.push_back(0.11);
			}
			if(one_life || four_lives) // one life
				energy = 1;
			// lambdas
			auto add_energy = [&](float addition) {
				if(one_life) {
					// one life
					if(addition < 0)
						energy = 0;
				}
				else if(four_lives) {
					// four lives
					if(addition < 0)
						energy -= 0.25;
				}
				else if(energy > 0)
					energy += addition;
				if(energy > 1)
					energy = 1;
				if(energy < 0)
					energy = 0;
			};
			auto update_multiplier = [&](bool good) {
				if(good) {
					progress++;
					if(multiplier < 8 && progress == multiplier*2) {
						progress = 0;
						multiplier *= 2;
					}
					return;
				}
				if(multiplier > 1)
					multiplier /= 2;
				progress = 0;
			};
			auto score_for_note = [](bsor::NoteEvent &note) -> int {
				int scoring_type = (note.id / 10000)-2;
				auto cut_data = note.cut_data.value();
				GlobalNamespace::ScoreModel::NoteScoreDefinition *def = scoring_type == -2
					? GlobalNamespace::ScoreModel::GetNoteScoreDefinition(GlobalNamespace::NoteData::ScoringType::Normal)
					: GlobalNamespace::ScoreModel::GetNoteScoreDefinition(scoring_type);
				return def->fixedCutScore
					+ int(std::lerp(def->minBeforeCutScore, def->maxBeforeCutScore, std::clamp(cut_data.before_cut_rating, 0.0f, 1.0f)) + 0.5)
					+ int(std::lerp(def->minAfterCutScore, def->maxAfterCutScore, std::clamp(cut_data.after_cut_rating, 0.0f, 1.0f)) + 0.5)
					+ int(def->maxCenterDistanceCutScore * (1 - std::clamp(cut_data.cut_dist_to_center / 0.3, 0.0, 1.0)) + 0.5);
			};
			auto energy_for_note = [](bsor::NoteEvent &note) -> float {
				if(note.type == bsor::NoteEvent::BOMB)
					return -0.15;
				int scoring_type = (note.id / 10000)-2;
				bool good = note.type == bsor::NoteEvent::GOOD_HIT;
				bool miss = note.type == bsor::NoteEvent::MISS;
				switch(scoring_type) {
					case -2:
					case (int)GlobalNamespace::NoteData::ScoringType::Normal:
						return good ? 0.01 : (miss ? -0.15 : -0.1);
					case (int)GlobalNamespace::NoteData::ScoringType::BurstSliderElement:
						return good ? 0.002 : (miss ? -0.03 : -0.025);
					default:
						return 0;
				}
			};
			for(auto &event : bsor->events) {
				// add wall change since last event
				if(last_wall != wall_end) {
					if(event.time < wall_end) {
						add_energy(1.3 * (last_wall - event.time));
					} else {
						add_energy(1.3 * (last_wall - wall_end));
						last_wall = wall_end;
					}
				}
				switch(event.type) {
					case bsor::EventType::NOTE: {
						auto &note = event.note;
						if(note.type == bsor::NoteEvent::GOOD_HIT) {
							update_multiplier(true);
							int s = score_for_note(note) * multiplier;
							score += s;
							for(float mod : modifiers) {
								score += std::round(s*mod);
							}
						} else {
							update_multiplier(false);
						}
						add_energy(energy_for_note(note));
						WRITE(note.time);
						break;
					}
					case bsor::EventType::WALL: {
						auto &wall = event.wall;
						if(wall.time > wall_end) {
							update_multiplier(false);
						}
						last_wall = wall.time;
						float end_time = wall.time + (energy - wall.energy)/1.3;
						wall_end = std::max(wall_end, end_time);
						WRITE(wall.time);
					}
					default:
						break;
				}
			}
			#undef HAS_MODIFIER
			#undef WRITE
			fclose(fp);
			return true;
		} else {
			PaperLogger.warn("Error encountered while parsing BSOR: {}", std::get<std::string>(result));
			return false;
		}
	}
}
