#pragma once

#include "config-utils/shared/config-utils.hpp"

namespace selfcompete {
	DECLARE_JSON_CLASS(Profile,
		VALUE(std::string, name);
		VALUE(std::string, id);
	);
}

DECLARE_CONFIG(ModConfig,
	CONFIG_VALUE(enabled, bool, "enabled", true);
	CONFIG_VALUE(selfcompete, bool, "selfcompete", true);
	CONFIG_VALUE(save_on_fail, bool, "save_on_fail", true);
	CONFIG_VALUE(save_on_exit, bool, "save_on_exit", false);
	CONFIG_VALUE(save_on_restart, bool, "save_on_restart", false);
	CONFIG_VALUE(beatleader_profiles, std::vector<selfcompete::Profile>, "beatleader_profiles", {});
)
