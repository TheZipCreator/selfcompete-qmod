#include "main.hpp"

#include "scotland2/shared/modloader.h"
#include "bsml/shared/BSML.hpp"

#include "menu.hpp"
#include "modconfig.hpp"
#include "selfcompete.hpp"

modloader::ModInfo mod_info{MOD_ID, VERSION, 0};
// Stores the ID and version of our mod, and is sent to
// the modloader upon startup

// Loads the config from disk using our mod_info, then returns it for use
// other config tools such as config-utils don't use this config, so it can be
// removed if those are in use
Configuration &getConfig() {
	static Configuration config(mod_info);
	return config;
}

// Called at the early stages of game loading
MOD_EXTERN_FUNC void setup(CModInfo *info) noexcept {
	*info = mod_info.to_c();

	getConfig().Load();

	// File logging
	Paper::Logger::RegisterFileContextId(PaperLogger.tag);

	PaperLogger.info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
MOD_EXTERN_FUNC void late_load() noexcept {
	il2cpp_functions::Init();

	getModConfig().Init(mod_info);
	
	BSML::Init();
	BSML::Register::RegisterSettingsMenu("SelfCompete", &selfcompete::did_activate, false);

	PaperLogger.info("Installing hooks...");
	selfcompete::register_hooks();
	PaperLogger.info("Installed all hooks!");

}
