#pragma once

#include "GlobalNamespace/BeatmapDifficulty.hpp"

#include "main.hpp"


#define RETURN_IF_NULL(VALUE, RETURN) do { \
	if(VALUE == nullptr) { \
		PaperLogger.warn(#VALUE " is null."); \
		return RETURN; \
	} \
} while(0)

namespace selfcompete {
	// converts a difficulty to a string
	std::string difficulty_to_string(GlobalNamespace::BeatmapDifficulty dif); 	
	// generates a random filename
	std::string generate_filename(void);
	// gets a temporary directory
	std::string get_tmp_dir(void);
	// converts a replay file to a score file. Returns `true` if successful
	bool convert_replay(std::string in_file, std::string out_file);
}
