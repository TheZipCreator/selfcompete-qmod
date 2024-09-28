#include "menu.hpp"

#include "modconfig.hpp"

namespace selfcompete {
	void did_activate(HMUI::ViewController *self, bool first_activation, bool added_to_hierarchy, bool screen_system_enabling) {
		if(!first_activation)
			return;
		auto container = BSML::Lite::CreateScrollableSettingsContainer(self->transform);
		BSML::Lite::CreateToggle(container->transform, "Enabled", getModConfig().enabled.GetValue(), [](bool enabled) {
			getModConfig().enabled.SetValue(enabled);
		});
	}
}
