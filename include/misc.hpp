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
}
