#include <thread>
#include <cstdio>

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "GlobalNamespace/StandardLevelDetailViewController.hpp"
#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"

#include "web-utils/shared/WebUtils.hpp"

#include "menu.hpp"
#include "modconfig.hpp"
#include "misc.hpp"
#include "main.hpp"
#include "selfcompete.hpp"

#define BEATLEADER_API std::string("https://api.beatleader.xyz")

static GlobalNamespace::BeatmapKey key;
static bool setup_download_ui = false;

MAKE_HOOK_MATCH(
	StandardLevelDetailViewController_ShowOwnedContent,
	&GlobalNamespace::StandardLevelDetailViewController::ShowOwnedContent,
	void,
	GlobalNamespace::StandardLevelDetailViewController *self
) {
	StandardLevelDetailViewController_ShowOwnedContent(self);
	key = self->beatmapKey;
	if(!setup_download_ui) {
		// mostly copied from PlaylistManager
		auto canvas = BSML::Lite::CreateCanvas()->GetComponent<UnityEngine::RectTransform*>();
		canvas->SetParent(self->_standardLevelDetailView->transform, false);
		canvas->localScale = {1, 1, 1};
		canvas->sizeDelta = {40, 60};
		canvas->anchoredPosition = {53, -53};
		auto download_replay_button = BSML::Lite::CreateUIButton(canvas, "R", "ActionButton", []() {
			auto screen = BSML::Lite::CreateFloatingScreen({100, 50}, {0, 1, 2}, {0, 0, 0});
			auto scroll = BSML::Lite::CreateScrollView(screen->transform);
			for(auto &profile : getModConfig().beatleader_profiles.GetValue()) {
				auto text = BSML::Lite::CreateText(scroll->transform, profile.name+" - Finding replay...", {0, 0}, {0, 4});
				auto level_id = static_cast<std::string>(key.levelId);
				auto hash = level_id.rfind("custom_level_", 0) != std::string::npos ? level_id.substr(13) : level_id; // there actually isn't even a better way to do this
				PaperLogger.debug("level_id: '{}' hash: '{}'", level_id, hash);
				auto difficulty = selfcompete::difficulty_to_string(key.difficulty);
				auto characteristic = static_cast<std::string>(key.beatmapCharacteristic->_serializedName);
				std::unordered_map<std::string, std::string> args = {
					{"count", "10000"} // honestly surprised they let you do this
				};
				#define CHANGE(MSG) \
				BSML::MainThreadScheduler::Schedule([=]() { \
					text->text = profile.name+" - "+MSG; \
				})
				WebUtils::GetAsync<WebUtils::JsonResponse>(WebUtils::URLOptions(BEATLEADER_API+"/v3/scores/"+hash+"/"+difficulty+"/"+characteristic+"/modifiers/global/page", args), [=](WebUtils::JsonResponse response) {
					if(response.httpCode != 200) {
						CHANGE("Server returned error.");
						return;
					}
					if(response.responseData == std::nullopt) {
						CHANGE("Couldn't parse JSON.");
						return;
					}
					auto document = std::move(response.responseData.value());
					auto data = document["data"].GetArray();
					auto it = std::find_if(data.begin(), data.end(), [=](auto &val) {
						return val["playerId"].GetString() == profile.id;
					});
					if(it == data.end()) {
						CHANGE("No replay for this level.");
						return;
					}
					CHANGE("Downloading replay...");
					WebUtils::GetAsync<WebUtils::DataResponse>(WebUtils::URLOptions((*it)["replay"].GetString()), [=](WebUtils::DataResponse response) {
						if(response.httpCode != 200) {
							CHANGE("Server returned error.");
							return;
						}
						if(response.responseData == std::nullopt) {
							CHANGE("Couldn't get replay.");
							return;
						}
						// write to temporary file
						auto data = response.responseData.value();
						std::string filename = selfcompete::get_tmp_dir()+"/"+selfcompete::generate_filename()+".bsor";
						FILE *fp = fopen(filename.c_str(), "wb");
						fwrite(data.data(), sizeof(uint8_t), data.size(), fp);
						fclose(fp);
						CHANGE("Converting replay...");
						if(selfcompete::convert_replay(filename, selfcompete::get_level_dir(level_id, characteristic, difficulty)+SELFCOMPETE_BEATLEADER_DIR+"/"+profile.id))
							CHANGE("Replay downloaded and converted.");
						else
							CHANGE("Conversion failed!");
						std::remove(filename.c_str()); // remove temporary bsor file
					}, [=](float progress) {
						std::stringstream stream;
						stream << "Downloading replay... " << std::setprecision(2) << std::fixed << progress*100 << "%";
						auto str = stream.str();
						CHANGE(str);
					});

				});
				#undef CHANGE
			}
			BSML::Lite::CreateUIButton(scroll->transform, "Close", [=]() {
				UnityEngine::Object::Destroy(screen->gameObject);
			});
		});
		download_replay_button->GetComponent<UnityEngine::UI::LayoutElement *>()->preferredWidth = 0;
		BSML::Lite::AddHoverHint(download_replay_button, "Downloads replays for SelfCompete");
		setup_download_ui = true;
	}
}

MAKE_HOOK_MATCH(
	StandardLevelDetailView_RefreshContent,
	&GlobalNamespace::StandardLevelDetailView::RefreshContent,
	void,
	GlobalNamespace::StandardLevelDetailView *self
) {
	StandardLevelDetailView_RefreshContent(self);
	key = self->beatmapKey;
}

namespace selfcompete {	
	
	void register_menu_hooks(void) {
		INSTALL_HOOK(PaperLogger, StandardLevelDetailViewController_ShowOwnedContent);	
		INSTALL_HOOK(PaperLogger, StandardLevelDetailView_RefreshContent);
	}

	void did_activate(HMUI::ViewController *self, bool first_activation, bool added_to_hierarchy, bool screen_system_enabling) {
		if(!first_activation)
			return;
		auto container = BSML::Lite::CreateScrollableSettingsContainer(self->transform);
		BSML::Lite::CreateToggle(container->transform, "Enabled", getModConfig().enabled.GetValue(), [](bool enabled) {
			getModConfig().enabled.SetValue(enabled);
		});
		BSML::Lite::AddHoverHint(BSML::Lite::CreateToggle(container->transform, "Compete with Own Scores", getModConfig().selfcompete.GetValue(), [](bool enabled) {
			getModConfig().selfcompete.SetValue(enabled);
		}), "Includes scores that you recorded (i.e. the ones that aren't from BeatLeader) in the ranking.");
		BSML::Lite::AddHoverHint(BSML::Lite::CreateToggle(container->transform, "Save on Fail", getModConfig().save_on_fail.GetValue(), [](bool enabled) {
			getModConfig().save_on_fail.SetValue(enabled);
		}), "Whether to save your score replay when you fail a map.");
		BSML::Lite::AddHoverHint(BSML::Lite::CreateToggle(container->transform, "Save on Exit", getModConfig().save_on_exit.GetValue(), [](bool enabled) {
			getModConfig().save_on_exit.SetValue(enabled);
		}), "Whether to save your score replay when you exit a map (by clicking 'Menu').");
		BSML::Lite::AddHoverHint(BSML::Lite::CreateToggle(container->transform, "Save on Restart", getModConfig().save_on_restart.GetValue(), [](bool enabled) {
			getModConfig().save_on_restart.SetValue(enabled);
		}), "Whether to save your score replay when you restart a map.");
		auto profile_gos = std::make_shared<std::vector<UnityW<UnityEngine::GameObject>>>(); 
		BSML::Lite::CreateText(container->transform, "Beatleader Profiles (these may be used to fetch replays from to play against)\nNOTE: This is case-sensitive.\nTo download replays, click the 'R' button when selecting a level.", {0, 0}, {0, 16});
		// adds a profile
		auto add_profile = [=](Profile &profile) {
			auto hgroup = BSML::Lite::CreateHorizontalLayoutGroup(container->transform)->gameObject;
			profile_gos->push_back(hgroup);
			BSML::Lite::CreateText(hgroup->transform, profile.name);
			auto id_text = BSML::Lite::CreateText(hgroup->transform, "Resolving ID...");
			#define ID_FAIL(MSG) do { \
				BSML::MainThreadScheduler::Schedule([=]() { \
					id_text->text = "Couldn't resolve ID:\n" MSG; \
				}); \
				return; \
			} while(0)
			if(profile.id != "") {
				id_text->text = profile.id;
			} else {
				// find id
				std::unordered_map<std::string, std::string> args = {
					{"search", profile.name}
				};
				WebUtils::GetAsync<WebUtils::JsonResponse>(WebUtils::URLOptions(BEATLEADER_API+"/players", args), [=](WebUtils::JsonResponse response) {
					if(response.httpCode != 200)
						ID_FAIL("Server returned error.");
					if(response.responseData == std::nullopt)
						ID_FAIL("Couldn't parse JSON.");
					auto document = std::move(response.responseData.value());
					auto data = document["data"].GetArray();
					auto it = std::find_if(data.begin(), data.end(), [=](auto &val) { 
						PaperLogger.debug("'{}' == '{}'", val["name"].GetString(), profile.name);
						return val["name"].GetString() == profile.name; 
					});
					if(it == data.end())
						ID_FAIL("Couldn't find user.");
					auto id = (*it)["id"].GetString();
					BSML::MainThreadScheduler::Schedule([=]() {
						id_text->text = id;
						auto profiles = getModConfig().beatleader_profiles.GetValue();
						auto profile_it = std::find(profiles.begin(), profiles.end(), profile);
						if(profile_it != profiles.end()) {
							profile_it->id = id;
							getModConfig().beatleader_profiles.SetValue(profiles);
						}
					});

				});
			}
			#undef ID_FAIL
			BSML::Lite::CreateUIButton(hgroup->transform, "Remove", [=]() {
				// remove game object
				auto go_it = std::find(profile_gos->begin(), profile_gos->end(), hgroup);
				if(go_it != profile_gos->end()) {
					UnityEngine::Object::Destroy(*go_it);
					profile_gos->erase(go_it);
				}
				// remove from config
				auto profiles = getModConfig().beatleader_profiles.GetValue();
				auto profile_it = std::find(profiles.begin(), profiles.end(), profile);
				if(profile_it != profiles.end()) {
					profiles.erase(profile_it);
					getModConfig().beatleader_profiles.SetValue(profiles);
				}
			});
		};
		// add profiles and add ability to add profiles
		{
			auto profile_name = BSML::Lite::CreateStringSetting(container->transform, "Profile Name", "");
			BSML::Lite::CreateUIButton(container->transform, "Add Profile", [=]() {
				auto name = profile_name->text;
				if(name == "")
					return;
				// check to make sure it doesn't already exist
				auto profiles = getModConfig().beatleader_profiles.GetValue();
				if(std::find_if(profiles.begin(), profiles.end(), [=](Profile &profile) { return profile.name == name; }) != profiles.end())
					return; // already in list
				// add
				auto profile = Profile();
				profile.name = static_cast<std::string>(name);
				profile.id = "";
				add_profile(profile);
				profiles.push_back(profile);
				getModConfig().beatleader_profiles.SetValue(profiles);
			});
		}
		for(auto &name : getModConfig().beatleader_profiles.GetValue()) {
			add_profile(name);
		}
	}
}
