#include "misc.hpp"

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
}
