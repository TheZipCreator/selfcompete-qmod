#pragma once

#include "bsml/shared/BSML.hpp"

namespace selfcompete {
	// BSML did_activate method
	void did_activate(HMUI::ViewController *self, bool first_activation, bool added_to_hierarchy, bool screen_system_enabling);
	// registers menu hooks
	void register_menu_hooks(void);
}
