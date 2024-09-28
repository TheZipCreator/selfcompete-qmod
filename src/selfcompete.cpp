#include <filesystem>

#include "UnityEngine/Object.hpp"
#include "UnityEngine/WaitForSecondsRealtime.hpp"

#include "GlobalNamespace/ScoreUIController.hpp"
#include "GlobalNamespace/IScoreController.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/GameSongController.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"

#include "System/Action_1.hpp"
#include "System/Collections/IEnumerator.hpp"

#include "TMPro/TextAlignmentOptions.hpp"

#include "bs-utils/shared/utils.hpp"
#include "bsml/shared/BSML.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/utils.h"

#include "selfcompete.hpp"
#include "misc.hpp"
#include "modconfig.hpp"
#include "main.hpp"

#define AFTER_SCENE_SWITCH_CALLBACK \
custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer *> *>(std::function([=](Zenject::DiContainer *arg) { \
	if(practiceSettings == nullptr) \
		selfcompete::activate(key); \
	if(afterSceneSwitchCallback != nullptr) { \
		auto array = Array<System::Object *>::New(arg); \
		afterSceneSwitchCallback->DynamicInvokeImpl(array); \
	} \
}))
#define LEVEL_FINISHED_CALLBACK \
custom_types::MakeDelegate<System::Action_2<UnityW<GlobalNamespace::StandardLevelScenesTransitionSetupDataSO>, GlobalNamespace::LevelCompletionResults*>*>( \
	std::function([=](UnityW<GlobalNamespace::StandardLevelScenesTransitionSetupDataSO> arg1, GlobalNamespace::LevelCompletionResults *arg2) { \
		selfcompete::deactivate(); \
		if(levelFinishedCallback != nullptr) { \
			auto array = Array<System::Object *>::New(arg1, arg2); \
			levelFinishedCallback->DynamicInvokeImpl(array); \
		} \
	}) \
) 
#define LEVEL_RESTARTED_CALLBACK \
custom_types::MakeDelegate<System::Action_2<UnityW<GlobalNamespace::LevelScenesTransitionSetupDataSO>, GlobalNamespace::LevelCompletionResults*>* >( \
	std::function([=](UnityW<GlobalNamespace::LevelScenesTransitionSetupDataSO> arg1, GlobalNamespace::LevelCompletionResults* arg2) { \
		selfcompete::deactivate(); \
		if(practiceSettings == nullptr) \
			selfcompete::activate(key); \
		if(levelRestartedCallback != nullptr) { \
			auto array = Array<System::Object *>::New(arg1, arg2); \
			levelRestartedCallback->DynamicInvokeImpl(array); \
		} \
	}) \
) 



// beatgames WHY????? if you can't count the number of arguments on your fingers then it's too many fucking arguments ffs
MAKE_HOOK_MATCH(MenuTransitionsHelper_StartStandardLevel1,
	// these don't follow my usual snake_case style, but I copied them form bs-cordl and I really don't feel like manually converting all of them
	static_cast<void(GlobalNamespace::MenuTransitionsHelper::*)(
		StringW,
		ByRef<::GlobalNamespace::BeatmapKey>,
		GlobalNamespace::BeatmapLevel*,
		GlobalNamespace::IBeatmapLevelData*,
		GlobalNamespace::OverrideEnvironmentSettings*,
		GlobalNamespace::ColorScheme*,
		GlobalNamespace::ColorScheme*,
		GlobalNamespace::GameplayModifiers*,
		GlobalNamespace::PlayerSpecificSettings*,
		GlobalNamespace::PracticeSettings*,
		GlobalNamespace::EnvironmentsListModel*,
		StringW,
		bool,
		bool,
		System::Action*,
		System::Action_1<Zenject::DiContainer*>*,
		System::Action_2<UnityW<GlobalNamespace::StandardLevelScenesTransitionSetupDataSO>, GlobalNamespace::LevelCompletionResults*>*,
		System::Action_2<UnityW<GlobalNamespace::LevelScenesTransitionSetupDataSO>, GlobalNamespace::LevelCompletionResults*>*,
		System::Nullable_1<GlobalNamespace::__RecordingToolManager__SetupData>
	)>(&GlobalNamespace::MenuTransitionsHelper::StartStandardLevel), void,
	
	GlobalNamespace::MenuTransitionsHelper *self,
	StringW gameMode, 
	ByRef<::GlobalNamespace::BeatmapKey> beatmapKey,
	GlobalNamespace::BeatmapLevel* beatmapLevel,
	GlobalNamespace::IBeatmapLevelData* beatmapLevelData,
	GlobalNamespace::OverrideEnvironmentSettings* overrideEnvironmentSettings,
	GlobalNamespace::ColorScheme* overrideColorScheme,
	GlobalNamespace::ColorScheme* beatmapOverrideColorScheme,
	GlobalNamespace::GameplayModifiers* gameplayModifiers,
	GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings,
	GlobalNamespace::PracticeSettings* practiceSettings,
	GlobalNamespace::EnvironmentsListModel* environmentsListModel,
	StringW backButtonText,
	bool useTestNoteCutSoundEffects,
	bool startPaused,
	System::Action* beforeSceneSwitchCallback,
	System::Action_1<Zenject::DiContainer*>* afterSceneSwitchCallback,
	System::Action_2<UnityW<GlobalNamespace::StandardLevelScenesTransitionSetupDataSO>, GlobalNamespace::LevelCompletionResults*>* levelFinishedCallback,
	System::Action_2<UnityW<GlobalNamespace::LevelScenesTransitionSetupDataSO>, GlobalNamespace::LevelCompletionResults*>* levelRestartedCallback,
	System::Nullable_1<GlobalNamespace::__RecordingToolManager__SetupData> recordingToolData
) {
	auto key = *beatmapKey;
	MenuTransitionsHelper_StartStandardLevel1(
		self,
		gameMode,
		beatmapKey,
		beatmapLevel,
		beatmapLevelData,
		overrideEnvironmentSettings,
		overrideColorScheme,
		beatmapOverrideColorScheme,
		gameplayModifiers,
		playerSpecificSettings,
		practiceSettings,
		environmentsListModel,
		backButtonText,
		useTestNoteCutSoundEffects,
		startPaused,
		beforeSceneSwitchCallback,
		AFTER_SCENE_SWITCH_CALLBACK,
		LEVEL_FINISHED_CALLBACK,
		LEVEL_RESTARTED_CALLBACK,
		recordingToolData
	);
}

MAKE_HOOK_MATCH(MenuTransitionsHelper_StartStandardLevel2,
	static_cast<void(GlobalNamespace::MenuTransitionsHelper::*)(
		StringW,
		ByRef<GlobalNamespace::BeatmapKey>,
		GlobalNamespace::BeatmapLevel*,
		GlobalNamespace::OverrideEnvironmentSettings*,
		GlobalNamespace::ColorScheme*,
		GlobalNamespace::ColorScheme*,
		GlobalNamespace::GameplayModifiers*,
		GlobalNamespace::PlayerSpecificSettings*,
		GlobalNamespace::PracticeSettings*,
		GlobalNamespace::EnvironmentsListModel*,
		StringW,
		bool,
		bool,
		System::Action* beforeSceneSwitchCallback, System::Action_1<Zenject::DiContainer*>*,
		System::Action_2<UnityW<GlobalNamespace::StandardLevelScenesTransitionSetupDataSO>, GlobalNamespace::LevelCompletionResults*>*,
		System::Action_2<UnityW<GlobalNamespace::LevelScenesTransitionSetupDataSO>, GlobalNamespace::LevelCompletionResults*>*,
		System::Nullable_1<GlobalNamespace::__RecordingToolManager__SetupData>
	)>(&GlobalNamespace::MenuTransitionsHelper::StartStandardLevel), void,
		GlobalNamespace::MenuTransitionsHelper *self,
		::StringW gameMode, 
		ByRef<::GlobalNamespace::BeatmapKey> beatmapKey, 
		::GlobalNamespace::BeatmapLevel* beatmapLevel,
		::GlobalNamespace::OverrideEnvironmentSettings* overrideEnvironmentSettings, 
		::GlobalNamespace::ColorScheme* overrideColorScheme,
		::GlobalNamespace::ColorScheme* beatmapOverrideColorScheme, 
		::GlobalNamespace::GameplayModifiers* gameplayModifiers,
		::GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings, 
		::GlobalNamespace::PracticeSettings* practiceSettings,
		::GlobalNamespace::EnvironmentsListModel* environmentsListModel, 
		::StringW backButtonText, 
		bool useTestNoteCutSoundEffects, 
		bool startPaused,
		::System::Action* beforeSceneSwitchCallback, 
		::System::Action_1<::Zenject::DiContainer*>* afterSceneSwitchCallback,
		::System::Action_2<::UnityW<::GlobalNamespace::StandardLevelScenesTransitionSetupDataSO>, ::GlobalNamespace::LevelCompletionResults*>* levelFinishedCallback,
		::System::Action_2<::UnityW<::GlobalNamespace::LevelScenesTransitionSetupDataSO>, ::GlobalNamespace::LevelCompletionResults*>* levelRestartedCallback,
		::System::Nullable_1<::GlobalNamespace::__RecordingToolManager__SetupData> recordingToolData
) {
	auto key = *beatmapKey;
	MenuTransitionsHelper_StartStandardLevel2(
		self,
		gameMode,
		beatmapKey,
		beatmapLevel,
		overrideEnvironmentSettings,
		overrideColorScheme,
		beatmapOverrideColorScheme,
		gameplayModifiers,
		playerSpecificSettings,
		practiceSettings,
		environmentsListModel,
		backButtonText,
		useTestNoteCutSoundEffects,
		startPaused,
		beforeSceneSwitchCallback,
		AFTER_SCENE_SWITCH_CALLBACK,
		LEVEL_FINISHED_CALLBACK,
		LEVEL_RESTARTED_CALLBACK,
		recordingToolData
	);
}

#undef AFTER_SCENE_SWITCH_CALLBACK
#undef LEVEL_FINISHED_CALLBACK
#undef LEVEL_RESTARTED_CALLBACK

MAKE_HOOK_MATCH(ScoreUIController_UpdateScore, &GlobalNamespace::ScoreUIController::UpdateScore, void,
	GlobalNamespace::ScoreUIController *self,
	int multipliedScore,
	int modifiedScore
) {
	ScoreUIController_UpdateScore(self, multipliedScore, modifiedScore);
	if(!selfcompete::state.activated)
		return;
	auto gsc = UnityEngine::Object::FindObjectOfType<GlobalNamespace::GameSongController *>();
	RETURN_IF_NULL(gsc,);
	selfcompete::update(gsc->_audioTimeSyncController->songTime, modifiedScore);
}

MAKE_HOOK_MATCH(PauseMenuManager_MenuButtonPressed, &GlobalNamespace::PauseMenuManager::MenuButtonPressed, void, GlobalNamespace::PauseMenuManager *self) {
	PauseMenuManager_MenuButtonPressed(self);
	selfcompete::deactivate();
}

namespace selfcompete {
	State state;

	void ScoreKeeper::update(float time, int score) {}
	
	ReadingScoreKeeper::ReadingScoreKeeper(FILE *fp) {
		this->fp = fp;
		timeRead = 0;
		scoreRead = 0;
		score = 0;
	}
	RecordingScoreKeeper::RecordingScoreKeeper(FILE *fp) {
		this->fp = fp;
		score = 0;
	}

	void ReadingScoreKeeper::update(float time, int score) {
		if(feof(fp))
			return;
		while(true) {
			if(feof(fp))
				break;
			fread(&timeRead, sizeof(float), 1, fp);
			fread(&scoreRead, sizeof(int), 1, fp);
			if(time <= timeRead) {
				this->score = scoreRead;
				break;
			}
		}
	}
	void RecordingScoreKeeper::update(float time, int score) {
		this->score = score;
		fwrite(&time, sizeof(float), 1, fp);
		fwrite(&score, sizeof(int), 1, fp);
	}

	ScoreKeeper::~ScoreKeeper() {
		PaperLogger.debug("~ScoreKeeper()");
		fclose(fp);
	}
	
	void register_hooks(void) {
		INSTALL_HOOK(PaperLogger, MenuTransitionsHelper_StartStandardLevel1);
		INSTALL_HOOK(PaperLogger, MenuTransitionsHelper_StartStandardLevel2);
		INSTALL_HOOK(PaperLogger, ScoreUIController_UpdateScore);
		INSTALL_HOOK(PaperLogger, PauseMenuManager_MenuButtonPressed);
	}
	
	void activate(GlobalNamespace::BeatmapKey key) {
		if(!getModConfig().enabled.GetValue() || !bs_utils::Submission::getEnabled()) {
			if(state.rank_text)
				state.rank_text->gameObject->active = false;
			return;
		} else {
			if(state.rank_text)
				state.rank_text->gameObject->active = true;
		}
		PaperLogger.info("Activating SelfCompete...");
		state.activated = true;
		if(!state.rank_text) {
			auto cnv = BSML::Lite::CreateCanvas();
			cnv->transform->position = UnityEngine::Vector3(0, 0, 4);
			cnv->transform->eulerAngles = UnityEngine::Vector3(90, 0, 0);
			state.rank_text.emplace(BSML::Lite::CreateText(cnv->transform, "? / ?", TMPro::FontStyles::Normal, 32.f, {0, 0}, {100, 10}));
			state.rank_text->alignment = TMPro::TextAlignmentOptions::Center;
		}	
		state.score_keepers.clear();
		// create score keepers
		std::string level_dir = 
			getDataDir(mod_info)+"/"+
			static_cast<std::string>(key.levelId)+"__"+
			static_cast<std::string>(key.beatmapCharacteristic->_serializedName)+"__"+
			difficulty_to_string(key.difficulty);
		if(!direxists(level_dir))
			mkpath(level_dir);
		// load existing files
		for(const auto &entry : std::filesystem::directory_iterator(level_dir)) {
			FILE *fp = fopen(entry.path().c_str(), "rb");
			if(fp == nullptr) {
				PaperLogger.warn("Could not open score file '{}'", entry.path().string());
				continue;
			}
			int fmt_version;
			fread(&fmt_version, sizeof(int), 1, fp);
			switch(fmt_version) {
				case 0: {
					state.score_keepers.push_back(std::unique_ptr<ScoreKeeper>(new ReadingScoreKeeper(fp)));
					break;
				}
				default:
					PaperLogger.warn("Unknown score file format version {}", fmt_version);
					break;
			}
		}
		// create recorder file
		// generate random name
		// (technically could clobber another file but that's incredibly unlikely so I'm not gonna bother checking)
		{
			std::string name(64, ' ');
			std::string acceptable_chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
			for(int i = 0; i < 64; i++) {
				name[i] = acceptable_chars[std::rand()%acceptable_chars.size()];
			}
			FILE *fp = fopen((level_dir+"/"+name).c_str(), "wb");
			if(fp == nullptr) {
				PaperLogger.warn("Could not create recorder file.");
				return;
			}
			int fmt_version = SELFCOMPETE_FORMAT_VERSION;
			fwrite(&fmt_version, sizeof(int), 1, fp);
			state.score_keepers.push_back(std::unique_ptr<ScoreKeeper>(new RecordingScoreKeeper(fp)));
			state.recorder_fp = fp;
		}
		PaperLogger.info("SelfCompete activated!");

		update(0, 0);
	}
	void deactivate(void) {
		if(!state.activated)
			return;
		state.activated = false;
		state.score_keepers.clear();
		state.recorder_fp = nullptr;
		PaperLogger.info("SelfCompete deactivated!");
	}
	void update(float time, int score) {
		PaperLogger.debug("time={} score={}", time, score);
		// update score keepers
		for(auto &sk : state.score_keepers) {
			sk->update(time, score);
			PaperLogger.debug("score: {}", sk->score);
		}
		// sort score keepers
		std::sort(state.score_keepers.begin(), state.score_keepers.end(), [](auto &a, auto &b) { return a->score > b->score; });
		// find recorder
		size_t index = static_cast<size_t>(-1);
		for(size_t i = 0; i < state.score_keepers.size(); i++) {
			if(state.score_keepers[i]->fp != state.recorder_fp)
				continue;
			index = i;
			break;
		}
		std::stringstream stream;
		stream << index+1;
		stream << " / ";
		stream << state.score_keepers.size();
		state.rank_text->text = stream.str();
	}
}


