#include "world_rhythm_game.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/input/input_manager.h"
#include "pathos/loader/imageloader.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/brush.h"
#include "pathos/overlay/label.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include "badger/math/minmax.h"
#include <sstream>

// -----------------------------------------------------------------------
// Debug configuration

#define DEBUG_AUTO_PLAY_MODE        0
#define TEMP_RECORD_SAVE_PATH       "rhythm_game_record_saved.txt"

// -----------------------------------------------------------------------
// Game constants

#define KEY_RECORDS_NUM_RESERVED    16384

#define LANE_COUNT                  _countof(gLaneDesc)
#define LANE_X0                     120
#define LANE_Y0                     100
#define LANE_SPACE_X                10
#define LANE_WIDTH                  90
#define LANE_HEIGHT                 800
// Additional height after crossline.
#define LANE_HEIGHT_EXTRA           60
#define LANE_LABEL_OFFSET_X         (LANE_WIDTH / 2 - 10)
#define LANE_LABEL_OFFSET_Y         20

#define JUDGE_DISPLAY_PERIOD        0.5f
#define JUDGE_TYPE_PERFECT          0
#define JUDGE_TYPE_GOOD             1
#define JUDGE_TYPE_MISS             2
#define JUDGE_COLOR                 vector3(1.0f, 1.0f, 0.1f)
#define JUDGE_COLOR_FADE            vector3(0.1f, 0.1f, 0.1f)

#define SCOREBOARD_WIDTH            400
#define SCOREBOARD_HEIGHT           200
#define SCOREBOARD_MARGIN_X         20
#define SCOREBOARD_OFFSET_X         80
#define SCORE_LABEL_Y0              100
#define SCORE_LABEL_SPACE_Y         60
#define PERFECT_LABEL_COLOR         vector3(1.0f, 1.0f, 1.0f)
#define GOOD_LABEL_COLOR            vector3(0.8f, 0.8f, 0.8f)
#define MISS_LABEL_COLOR            vector3(1.0f, 0.5f, 0.5f)

// Delay in seconds before music starts when entering the play stage.
#define PLAY_START_DELAY            3.0f
// Time of seconds between appearing at the top and reaching at the bottom of lane.
#define KEY_DROP_PERIOD             0.8f
#define CATCH_RATIO_PERFECT         0.05f
#define CATCH_RATIO_GOOD            0.1f

// Input is determined as short note
// if the time diff between key press and release is less than this value.
#define SHORT_NOTE_PERIOD           0.2f

#define NOTE_WIDTH                  LANE_WIDTH
#define NOTE_HEIGHT                 40
#define NOTE_OBJECT_POOL_SIZE       100
#define NOTE_COLOR_BLUE             0
#define NOTE_COLOR_YELLOW           1

#define CROSSLINE_HEIGHT            NOTE_HEIGHT
#define CROSSLINE_COLOR             vector4(0.9f, 0.1f, 0.1f, 0.3f)

#define PRESS_EFFECT_WIDTH          LANE_WIDTH
#define PRESS_EFFECT_HEIGHT         120

#define CATCH_EFFECT_WIDTH          (1.5f * LANE_WIDTH)
#define CATCH_EFFECT_HEIGHT         (1.5f * LANE_WIDTH)
#define CATCH_EFFECT_PERIOD         0.25f

#define BROWSER_X0                  100
#define BROWSER_Y0                  100
#define BROWSER_ITEM_X0             20
#define BROWSER_ITEM_Y0             60
#define BROWSER_ITEM_SPACE_Y        40
#define BROWSER_ITEM_DEFAULT_COLOR  vector3(1.0f, 1.0f, 1.0f)
#define BROWSER_ITEM_SELECTED_COLOR vector3(1.0f, 0.7f, 0.2f)

#define NOTICE_NO_DATABASE          0

#define DEFAULT_MUSIC_VOLUME        0.5f

// Game resource files
#define GAME_RESOURCE_DIR           "../../resources/rhythm_game/"
#define GAME_EXTERNAL_RESOURCE_DIR  "../../resources_external/rhythm_game/"
#define MUSIC_DATABASE              "rhythm_game/music_database.txt"
#define MUSIC_DATABASE_OVERRIDE     "rhythm_game/music_database_override.txt"
#define BLUE_NOTE_IMAGE             "rhythm_game/note_blue.png"
#define YELLOW_NOTE_IMAGE           "rhythm_game/note_yellow.png"
#define PRESS_EFFECT_IMAGE          "rhythm_game/press_effect.png"
const char* CATCH_EFFECT_IMAGES[] = {
	"rhythm_game/catch_effect_0.png",
	"rhythm_game/catch_effect_1.png",
	"rhythm_game/catch_effect_2.png",
	"rhythm_game/catch_effect_3.png",
	"rhythm_game/catch_effect_4.png",
};

// -----------------------------------------------------------------------
// Gameplay

struct LaneDesc {
	std::wstring displayLabel;
	std::string inputEventName;
	ButtonBinding inputBinding;
	int32 noteColor;
};

static LaneDesc gLaneDesc[] = {
	{ L"A", "lane0", ButtonBinding({InputConstants::KEYBOARD_A }), NOTE_COLOR_BLUE   },
	{ L"S", "lane1", ButtonBinding({InputConstants::KEYBOARD_S }), NOTE_COLOR_YELLOW },
	{ L"D", "lane2", ButtonBinding({InputConstants::KEYBOARD_D }), NOTE_COLOR_BLUE   },
	{ L"J", "lane3", ButtonBinding({InputConstants::KEYBOARD_J }), NOTE_COLOR_BLUE   },
	{ L"K", "lane4", ButtonBinding({InputConstants::KEYBOARD_K }), NOTE_COLOR_YELLOW },
	{ L"L", "lane5", ButtonBinding({InputConstants::KEYBOARD_L }), NOTE_COLOR_BLUE   },
};

float getLaneX(int32 laneIndex) {
	return (float)LANE_X0 + laneIndex * (LANE_SPACE_X + LANE_WIDTH);
}
float getShortNoteY(float ratio) {
	return (LANE_Y0 - NOTE_HEIGHT / 2) + ratio * LANE_HEIGHT;
}
void getLongNoteTransform(
	float currT, float pressT, float releaseT,
	float* y, float* height)
{
	float kTop = 1.0f - badger::clamp(0.0f, (releaseT - currT) / KEY_DROP_PERIOD, 1.0f);
	float kBottom = 1.0f - badger::clamp(0.0f, (pressT - currT) / KEY_DROP_PERIOD, 1.0f);
	*y = LANE_Y0 + kTop * LANE_HEIGHT;
	*height = (badger::max)(1.0f, (kBottom - kTop) * LANE_HEIGHT);
}

class LaneNote : public pathos::Rectangle {
public:
	LaneNote() : Rectangle(NOTE_WIDTH, NOTE_HEIGHT) {}
	void setEventIndex(int32 inEventIndex) { eventIndex = inEventIndex; }
	int32 getEventIndex() const { return eventIndex; }
	void setCatched(bool value) { bCatched = value; }
	bool getCatched() const { return bCatched; }
private:
	int32 eventIndex = -1; // local index in lane
	bool bCatched = false;
};

class CatchEffect : public pathos::Rectangle {
public:
	CatchEffect(const std::vector<GLuint>& inEffectTextures)
		: Rectangle(CATCH_EFFECT_WIDTH, CATCH_EFFECT_HEIGHT)
		, effectTextures(inEffectTextures)
	{
		effectBrush = new ImageBrush(effectTextures[0]);
		setBrush(effectBrush);
	}
	void playEffect(float currentTime) {
		startTime = currentTime;
	}
	void updateEffect(float currentTime) {
		if (currentTime - startTime <= CATCH_EFFECT_PERIOD) {
			float k = (currentTime - startTime) / CATCH_EFFECT_PERIOD;
			uint32 numTextures = (uint32)effectTextures.size();
			uint32 ix = (uint32)(k * (float)numTextures);
			ix = badger::clamp(0u, ix, numTextures - 1);
			effectBrush->setTexture(effectTextures[ix]);
			setVisible(true);
		} else {
			setVisible(false);
		}
	}
	void stopEffect() {
		startTime = -1000.0f;
		setVisible(false);
	}
private:
	ImageBrush* effectBrush = nullptr;
	std::vector<GLuint> effectTextures;
	float startTime = -1000.0f;
};

class MusicListItemWidget : public pathos::Label {};

class MusicBrowserWidget : public DisplayObject2D {
	
public:
	MusicBrowserWidget() {
		headerLabel = new pathos::Label;
		headerLabel->setText(L"Select a music with arrow keys and space bar");
		headerLabel->setColor(vector3(1.0f, 1.0f, 0.2f));
		addChild(headerLabel);

		selector = new pathos::Label(L">");
		selector->setVisible(false);
		selector->setColor(BROWSER_ITEM_SELECTED_COLOR);
		addChild(selector);
	}

	void showNotice(int32 noticeCode) {
		headerLabel->setColor(vector3(1.0f, 0.1f, 0.1f));

		wchar_t msg[256];
		if (noticeCode == NOTICE_NO_DATABASE) {
			swprintf_s(msg, L"ERROR: Can't open resources/%S", MUSIC_DATABASE);
			headerLabel->setText(msg);
		}
	}

	void addItem(const std::string& title) {
		MusicListItemWidget* item = new MusicListItemWidget;
		
		std::wstring wTitle;
		MBCS_TO_WCHAR(title, wTitle);
		item->setText(wTitle.c_str());
		item->setX(BROWSER_ITEM_X0);
		item->setY(BROWSER_ITEM_Y0 + (float)(items.size() * BROWSER_ITEM_SPACE_Y));

		addChild(item);
		items.push_back(item);
	}

	void selectItem(int32 targetIx) {
		selectedIndex = badger::clamp(0, targetIx, (int32)items.size() - 1);
		selector->setVisible(true);
		selector->setX(0.0f);
		selector->setY(BROWSER_ITEM_Y0 + (float)selectedIndex * BROWSER_ITEM_SPACE_Y);
		for (size_t i = 0; i < items.size(); ++i) {
			items[i]->setColor(selectedIndex == i ? BROWSER_ITEM_SELECTED_COLOR : BROWSER_ITEM_DEFAULT_COLOR);
		}
	}

	int32 getSelectedIndex() const { return selectedIndex; }

private:
	pathos::Label* headerLabel = nullptr;

	int32 selectedIndex = -1;
	pathos::Label* selector = nullptr;
	std::vector<MusicListItemWidget*> items;

};

void loadMusicRecord(std::istream& archive, PlayRecord& outRecord) {
	int32 laneIndex = -1;
	float pressTime = -1.0f;
	float releaseTime = -1.0f;

	outRecord.clearRecord(LANE_COUNT, KEY_RECORDS_NUM_RESERVED);

	std::string line;
	while (std::getline(archive, line)) {
		if (line.size() == 0) {
			continue;
		}
		std::stringstream ss(line);
		ss >> laneIndex >> pressTime >> releaseTime;
		if (releaseTime > 0.0f) {
			outRecord.addLongNoteEvent(laneIndex, pressTime, releaseTime);
		} else {
			outRecord.addShortNoteEvent(laneIndex, pressTime);
		}
	}

	outRecord.finalizeEvents();
}

void saveMusicRecord(LogFileWriter& fileWriter, const PlayRecord& playRecord, bool binaryFormat) {
	// #todo-rhythm: Support binary format
	CHECK(binaryFormat == false);

	char buf[1024];
	for (uint32 laneIx = 0; laneIx < LANE_COUNT; ++laneIx) {
		for (const LaneKeyEvent& evt : playRecord.getLaneEvents(laneIx)) {
			sprintf_s(buf, "%d %f %f", evt.laneIndex, evt.pressTime, evt.releaseTime);
			fileWriter.writeLine(buf);
		}
	}

	fileWriter.flush();
}

// -----------------------------------------------------------------------
// World_RhythmGame

void World_RhythmGame::onInitialize() {
	ResourceFinder::get().add(GAME_RESOURCE_DIR);
	ResourceFinder::get().add(GAME_EXTERNAL_RESOURCE_DIR);
	BassWrapper::initializeBASS();

	if (gConsole->isVisible()) {
		gConsole->toggle();
	}

	bool bValidMusicDatabase = loadMusicDatabase(MUSIC_DATABASE, MUSIC_DATABASE_OVERRIDE, musicDatabase);
	if (bValidMusicDatabase) {
		LOG(LogDebug, "Load database: %u items", musicDatabase.numItems());
	}

	musicVolume = DEFAULT_MUSIC_VOLUME;
	gEngine->registerConsoleCommand("music_volume", [this](const std::string& command) {
		float vol;
		int ret = sscanf_s(command.c_str(), "music_volume %f", &vol);
		if (ret != 1) {
			gConsole->addLine(L"Usage: music_volume vol", false, true);
		} else {
			this->setMusicVolume(vol);
			musicVolume = vol;
		}
	});

	// Input
	inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	for (size_t i = 0; i < LANE_COUNT; ++i) {
		const char* evtName = gLaneDesc[i].inputEventName.c_str();
		ButtonBinding& btnBinding = gLaneDesc[i].inputBinding;
		inputManager->bindButtonPressed(evtName, btnBinding, [i, this]() {
			if (!gConsole->isVisible()) {
				this->onPressLaneKey((int32)i);
			}
		});
		inputManager->bindButtonReleased(evtName, btnBinding, [i, this]() {
			if (!gConsole->isVisible()) {
				this->onReleaseLaneKey((int32)i);
			}
		});
	}
	inputManager->bindButtonPressed("browsePrevMusic", ButtonBinding({ InputConstants::KEYBOARD_ARROW_UP }), [this]() {
		this->browseMusicList(-1);
	});
	inputManager->bindButtonPressed("browseNextMusic", ButtonBinding({ InputConstants::KEYBOARD_ARROW_DOWN }), [this]() {
		this->browseMusicList(1);
	});
	inputManager->bindButtonPressed("startMusic", ButtonBinding({ InputConstants::SPACE }), [this]() {
		this->startPlaySession();
	});
	inputManager->bindButtonPressed("exitMusic", ButtonBinding({ InputConstants::ESC }), [this]() {
		this->exitPlaySession();
	});

	// Record (save)
	laneKeyPressTimes.resize(LANE_COUNT, -1.0f);
	playRecordFileWriter.initialize(TEMP_RECORD_SAVE_PATH);
	recordToSave.clearRecord(LANE_COUNT, KEY_RECORDS_NUM_RESERVED);
	gEngine->registerConsoleCommand("dump_play_record", [this](const std::string& command) {
		auto& fileWriter = this->playRecordFileWriter;
		auto& record = this->recordToSave;

		record.finalizeEvents();
		saveMusicRecord(fileWriter, record, false);

		wchar_t msg[256];
		swprintf_s(msg, L"Dumped %u events to: %S",
			record.getTotalLaneKeyEvents(),
			fileWriter.getFilepath().c_str());
		gConsole->addLine(msg, false, true);
	});

	// Graphics
	initializePlayStage();
	initializeBrowseStage();
	browserWidget->setVisible(true);
	playContainer->setVisible(false);

	if (!bValidMusicDatabase) {
		browserWidget->showNotice(NOTICE_NO_DATABASE);
	}
}

void World_RhythmGame::onDestroy() {
	BassWrapper::destroyBASS();
}

void World_RhythmGame::onTick(float deltaSeconds) {
	if (gameState == GameState::PlaySession) {
		currentGameTime = gEngine->getWorldTime() - initGameTime;
		
		// #todo-rhythm: More exact timing
		if (currentGameTime >= 0.0f && bMusicStarted == false) {
			musicStream->startPlay();
			bMusicStarted = true;
			countdownLabel->setVisible(false);
		} else {
			int32 countdown = (int32)(-currentGameTime) + 1;
			wchar_t msg[32];
			swprintf_s(msg, L"%d", countdown);
			countdownLabel->setText(msg);
			countdownLabel->setVisible(!bMusicStarted);
		}

#if DEBUG_AUTO_PLAY_MODE
		// Gather notes that should be pressed now and invoke onPressLaneKey().
		std::vector<int32> delayedReleaseLanes;
		for (uint32 laneIx = 0; laneIx < LANE_COUNT; ++laneIx) {
			const auto& laneEvents = loadedRecord.getLaneEvents(laneIx);
			int32 searchStartIndex = autoPlaySearchStartIndex[laneIx];
			int32 searchEndIndex = (int32)laneEvents.size();
			for (int32 eventIndex = searchStartIndex; eventIndex < searchEndIndex; ++eventIndex) {
				bool catchAny = false;
				const LaneKeyEvent& evt = laneEvents[eventIndex];

				float ratio = std::abs((evt.pressTime - currentGameTime) / KEY_DROP_PERIOD);
				bool bPerfect = (ratio <= CATCH_RATIO_PERFECT);

				if (bPerfect) {
					onPressLaneKey(evt.laneIndex);
					catchAny = true;
				}
				if (evt.isShortNote()) {
					if (catchAny) {
						delayedReleaseLanes.push_back(evt.laneIndex);
					}
				} else {
					float ratio = std::abs((evt.releaseTime - currentGameTime) / KEY_DROP_PERIOD);
					bool bPerfect = (ratio <= CATCH_RATIO_PERFECT);
					if (bPerfect) {
						onReleaseLaneKey(evt.laneIndex);
						catchAny = true;
					}
				}
				if (catchAny) {
					if (searchStartIndex != eventIndex) {
						LOG(LogDebug, "Update start index: %d", autoPlaySearchStartIndex);
					}
					autoPlaySearchStartIndex[laneIx] = eventIndex;
					break;
				}
			}
		}
#endif

		updateNotes(currentGameTime);

#if DEBUG_AUTO_PLAY_MODE
		for (int32 delayedReleaseLane : delayedReleaseLanes) {
			onReleaseLaneKey(delayedReleaseLane);
		}
#endif
	}
}

void World_RhythmGame::initializeBrowseStage() {
	browserWidget = new MusicBrowserWidget;
	browserWidget->setX(BROWSER_X0);
	browserWidget->setY(BROWSER_Y0);
	gEngine->getOverlayRoot()->addChild(browserWidget);

	const uint32 musicCount = musicDatabase.numItems();
	for (uint32 i = 0; i < musicCount; ++i) {
		browserWidget->addItem(musicDatabase.items[i].title);
	}
	if (musicCount > 0) {
		browserWidget->selectItem(0);
	}
}

void World_RhythmGame::initializePlayStage() {
	playContainer = new DisplayObject2D;
	gEngine->getOverlayRoot()->addChild(playContainer);

	background = new pathos::Rectangle(
		(float)gEngine->getConfig().windowWidth,
		(float)gEngine->getConfig().windowHeight);
	backgroundFallbackBrush = new pathos::SolidColorBrush(0.1f, 0.1f, 0.2f);
	backgroundImageBrush = new pathos::ImageBrush(gEngine->getSystemTexture2DBlack());
	background->setBrush(backgroundFallbackBrush);
	playContainer->addChild(background);

	auto laneBrush = new pathos::SolidColorBrush(0.1f, 0.1f, 0.1f);

	laneNoteColumns.resize(LANE_COUNT);
	lanePressEffects.reserve(LANE_COUNT);
	laneCatchffects.reserve(LANE_COUNT);
	noteBrushes.reserve(LANE_COUNT);

	// note brushes
	pathos::Brush* blueNoteBrush = nullptr;
	pathos::Brush* yellowNoteBrush = nullptr;
	{
		auto blueBlob = pathos::loadImage(BLUE_NOTE_IMAGE);
		if (blueBlob != nullptr) {
			GLuint blueNoteTexture = pathos::createTextureFromBitmap(blueBlob, false, false, "blue_note", true);
			blueNoteBrush = new pathos::ImageBrush(blueNoteTexture);
		} else {
			blueNoteBrush = new pathos::SolidColorBrush(0.8f, 0.8f, 1.0f);
		}
		auto yellowBlob = pathos::loadImage(YELLOW_NOTE_IMAGE);
		if (yellowBlob != nullptr) {
			GLuint yellowNoteTexture = pathos::createTextureFromBitmap(yellowBlob, false, false, "yellow_note", true);
			yellowNoteBrush = new pathos::ImageBrush(yellowNoteTexture);
		} else {
			yellowNoteBrush = new pathos::SolidColorBrush(1.0f, 1.0f, 0.2f);
		}
	}

	pathos::Brush* pressEffectBrush = nullptr;
	{
		auto effectBlob = pathos::loadImage(PRESS_EFFECT_IMAGE);
		if (effectBlob != nullptr) {
			GLuint effectTexture = pathos::createTextureFromBitmap(effectBlob, false, false, "note_press_effect", true);
			pressEffectBrush = new pathos::ImageBrush(effectTexture);
		} else {
			pressEffectBrush = new pathos::SolidColorBrush(0.1f, 0.9f, 0.1f);
		}
	}

	bCatchEffectAllValid = true;
	std::vector<pathos::BitmapBlob*> catchEffectBlobs;
	std::vector<GLuint> catchEffectTextures;
	for (size_t i = 0; i < _countof(CATCH_EFFECT_IMAGES); ++i) {
		auto catchEffectBlob = pathos::loadImage(CATCH_EFFECT_IMAGES[i]);
		if (catchEffectBlob != nullptr) {
			catchEffectBlobs.push_back(catchEffectBlob);
		} else {
			bCatchEffectAllValid = false;
			break;
		}
	}
	if (bCatchEffectAllValid == false) {
		for (pathos::BitmapBlob* blob : catchEffectBlobs) {
			delete blob;
		}
	} else {
		int32 i = 0;
		for (pathos::BitmapBlob* blob : catchEffectBlobs) {
			char msg[64];
			sprintf_s(msg, "catch_effect_%d", i++);
			GLuint effectTexture = pathos::createTextureFromBitmap(blob, false, false, msg, true);
			catchEffectTextures.push_back(effectTexture);
		}
	}
	
	// Lane columns
	std::vector<pathos::Rectangle*> laneColumnRects;
	for (uint32 laneIndex = 0; laneIndex < LANE_COUNT; ++laneIndex) {
		pathos::Rectangle* laneColumn = new pathos::Rectangle(LANE_WIDTH, LANE_HEIGHT + LANE_HEIGHT_EXTRA);
		laneColumn->setX(getLaneX((int32)laneIndex));
		laneColumn->setY((float)LANE_Y0);
		laneColumn->setBrush(laneBrush);
		playContainer->addChild(laneColumn);

		laneColumnRects.push_back(laneColumn);
	}

	auto crosslineBrush = new pathos::SolidColorBrush(CROSSLINE_COLOR);
	pathos::Rectangle* crossline = new pathos::Rectangle((LANE_WIDTH + LANE_SPACE_X) * LANE_COUNT - LANE_SPACE_X, CROSSLINE_HEIGHT);
	crossline->setX(LANE_X0);
	crossline->setY(LANE_Y0 + LANE_HEIGHT - CROSSLINE_HEIGHT / 2);
	crossline->setBrush(crosslineBrush);
	playContainer->addChild(crossline);

	for (uint32 laneIndex = 0; laneIndex < LANE_COUNT; ++laneIndex) {
		auto laneColumn = laneColumnRects[laneIndex];

		pathos::Rectangle* pressEffect = new pathos::Rectangle(PRESS_EFFECT_WIDTH, PRESS_EFFECT_HEIGHT);
		pressEffect->setX(laneColumn->getX());
		pressEffect->setY(laneColumn->getY() + LANE_HEIGHT - PRESS_EFFECT_HEIGHT);
		pressEffect->setBrush(pressEffectBrush);
		pressEffect->setVisible(false);
		playContainer->addChild(pressEffect);
		lanePressEffects.push_back(pressEffect);

		if (bCatchEffectAllValid) {
			CatchEffect* catchEffect = new CatchEffect(catchEffectTextures);
			catchEffect->setX(laneColumn->getX() + (LANE_WIDTH - CATCH_EFFECT_WIDTH) / 2);
			catchEffect->setY(laneColumn->getY() + LANE_HEIGHT - CATCH_EFFECT_HEIGHT / 2);
			catchEffect->setVisible(false);
			playContainer->addChild(catchEffect);
			laneCatchffects.push_back(catchEffect);
		}

		const wchar_t* labelText = gLaneDesc[laneIndex].displayLabel.c_str();
		pathos::Label* laneLabel = new pathos::Label(labelText);
		laneLabel->setX(laneColumn->getX() + LANE_LABEL_OFFSET_X);
		laneLabel->setY(laneColumn->getY() + LANE_HEIGHT + LANE_HEIGHT_EXTRA + LANE_LABEL_OFFSET_Y);
		laneLabel->setFont("defaultLarge");
		playContainer->addChild(laneLabel);

		auto noteBrush = (gLaneDesc[laneIndex].noteColor == NOTE_COLOR_BLUE)
			? blueNoteBrush
			: yellowNoteBrush;
		noteBrushes.push_back(noteBrush);
	}

	noteObjectPool.reserve(NOTE_OBJECT_POOL_SIZE);
	for (size_t i = 0; i < NOTE_OBJECT_POOL_SIZE; ++i) {
		noteObjectPool.push_back(new LaneNote());
	}

	noteContainer = new DisplayObject2D;
	playContainer->addChild(noteContainer);

	// Place the judge label in front of notes.
	judgeLabel = new pathos::Label(L"PERFECT");
	judgeLabel->setX(LANE_X0 - 90.0f + 0.5f * LANE_COUNT * (LANE_WIDTH + LANE_SPACE_X));
	judgeLabel->setY(0.5f * (LANE_Y0 + LANE_HEIGHT));
	judgeLabel->setColor(JUDGE_COLOR);
	judgeLabel->setFont("defaultLarge");
	judgeLabel->setVisible(false);
	playContainer->addChild(judgeLabel);

	countdownLabel = new pathos::Label(L"0");
	countdownLabel->setX(LANE_X0 - 90.0f + 0.5f * LANE_COUNT * (LANE_WIDTH + LANE_SPACE_X));
	countdownLabel->setY(0.5f * (LANE_Y0 + LANE_HEIGHT));
	countdownLabel->setColor(JUDGE_COLOR);
	countdownLabel->setFont("defaultLarge");
	playContainer->addChild(countdownLabel);

	scoreboardRect = new pathos::Rectangle(SCOREBOARD_WIDTH + 2 * SCOREBOARD_MARGIN_X, SCOREBOARD_HEIGHT);
	scoreboardRect->setX(getLaneX(LANE_COUNT + 1) + SCOREBOARD_OFFSET_X - SCOREBOARD_MARGIN_X);
	scoreboardRect->setY(SCORE_LABEL_Y0);
	scoreboardRect->setBrush(new SolidColorBrush(0.1f, 0.1f, 0.1f));
	playContainer->addChild(scoreboardRect);

	perfectLabel = new pathos::Label(L"PERFECT : 0");
	goodLabel    = new pathos::Label(L"GOOD    : 0");
	missLabel    = new pathos::Label(L"MISS    : 0");
	perfectLabel->setX(getLaneX(LANE_COUNT + 1) + SCOREBOARD_OFFSET_X);
	goodLabel->setX(perfectLabel->getX());
	missLabel->setX(perfectLabel->getX());
	perfectLabel->setY(SCORE_LABEL_Y0);
	goodLabel->setY(SCORE_LABEL_Y0 + 1 * SCORE_LABEL_SPACE_Y);
	missLabel->setY(SCORE_LABEL_Y0 + 2 * SCORE_LABEL_SPACE_Y);
	perfectLabel->setFont("defaultLarge");
	goodLabel->setFont("defaultLarge");
	missLabel->setFont("defaultLarge");
	perfectLabel->setColor(PERFECT_LABEL_COLOR);
	goodLabel->setColor(GOOD_LABEL_COLOR);
	missLabel->setColor(MISS_LABEL_COLOR);
	playContainer->addChild(perfectLabel);
	playContainer->addChild(goodLabel);
	playContainer->addChild(missLabel);
}

void World_RhythmGame::startPlaySession() {
	if (gameState != GameState::BrowseMusic) {
		return;
	}

	int32 ix = browserWidget->getSelectedIndex();
	if (ix == -1) {
		return;
	}

	MusicDatabaseItem& item = musicDatabase.items[ix];

	std::string mp3Path = ResourceFinder::get().find(item.musicFile);
	if (mp3Path.size() == 0) {
		LOG(LogError, "Failed to open mp3 file: %s", item.musicFile.c_str());
		return;
	}

	// Record (load)
	std::string recordPath = ResourceFinder::get().find(item.recordFile);
	if (recordPath.size() == 0) {
		LOG(LogError, "Failed to open record file: %s", item.recordFile.c_str());
		return;
	}
	std::fstream archive(recordPath, std::ios::in);
	if (archive.is_open() == false) {
		LOG(LogError, "Failed to load record: %s", recordPath.c_str());
		return;
	}
	loadMusicRecord(archive, loadedRecord);
	LOG(LogDebug, "Load record (%u): %s", loadedRecord.getTotalLaneKeyEvents(), recordPath.c_str());

	// Record (save)
	recordToSave.clearRecord(LANE_COUNT, KEY_RECORDS_NUM_RESERVED);
	laneKeyPressTimes.clear();
	laneKeyPressTimes.resize(LANE_COUNT, -1.0f);

	// Load background image
	std::string backgroundPath = ResourceFinder::get().find(item.backgroundFile);
	if (backgroundPath.size() == 0) {
		background->setBrush(backgroundFallbackBrush);
		LOG(LogError, "Failed to find background: %s", item.backgroundFile.c_str());
	} else {
		auto imageBlob = pathos::loadImage(backgroundPath.c_str());
		if (imageBlob == nullptr) {
			background->setBrush(backgroundFallbackBrush);
			LOG(LogError, "Failed to load background: %s", backgroundPath.c_str());
		} else {
			GLuint oldTexture = backgroundImageBrush->getTexture();
			if (oldTexture != 0) {
				ENQUEUE_RENDER_COMMAND([oldTexture](RenderCommandList& cmdList) {
					cmdList.deleteTextures(1, &oldTexture);
				});
			}

			GLuint imageTexture = pathos::createTextureFromBitmap(imageBlob, false, false, "background_image", true);
			backgroundImageBrush->setTexture(imageTexture);
			background->setBrush(backgroundImageBrush);
		}
	}

	musicStream = gBass->createStreamFromFile(mp3Path.c_str(), musicVolume);
	if (musicStream == nullptr) {
		return;
	}

	browserWidget->setVisible(false);
	playContainer->setVisible(true);

	// Initialize play data
	scoreboardData.clearScore();
	lastSearchedEventIndex.clear();
	lastSearchedEventIndex.resize(LANE_COUNT, 0);
#if DEBUG_AUTO_PLAY_MODE
	autoPlaySearchStartIndex.clear();
	autoPlaySearchStartIndex.resize(LANE_COUNT, 0);
#endif
	initGameTime = gEngine->getWorldTime() + PLAY_START_DELAY;
	gameState = GameState::PlaySession;
}

void World_RhythmGame::exitPlaySession() {
	if (gameState != GameState::PlaySession) {
		return;
	}

	if (musicStream != nullptr) {
		delete musicStream;
		musicStream = nullptr;
	}

	for (auto& column : laneNoteColumns) {
		for (size_t i = 0; i < column.size(); ++i) {
			LaneNote* note = column[i];

			returnNoteToPool(note);
			noteContainer->removeChild(note);
		}
		column.clear();
	}

	setJudge(-100.0f, JUDGE_TYPE_PERFECT);

	for (auto effect : lanePressEffects) {
		effect->setVisible(false);
	}
	for (auto effect : laneCatchffects) {
		effect->stopEffect();
	}

	browserWidget->setVisible(true);
	playContainer->setVisible(false);

	bMusicStarted = false;

	gameState = GameState::BrowseMusic;
}

void World_RhythmGame::updateNotes(float currT) {
	for (uint32 laneIx = 0; laneIx < LANE_COUNT; ++laneIx) {
		const std::vector<LaneKeyEvent>& laneEvents = loadedRecord.getLaneEvents(laneIx);
		for (size_t eventIndex = lastSearchedEventIndex[laneIx]; eventIndex < laneEvents.size(); ++eventIndex) {
			const LaneKeyEvent& evt = laneEvents[eventIndex];
			// Spawn new notes.
			if (evt.pressTime - KEY_DROP_PERIOD <= currT && currT <= evt.pressTime) {
				std::vector<LaneNote*>& column = laneNoteColumns[evt.laneIndex];
				bool shouldSpawn = true;
				for (LaneNote* note : column) {
					if (note->getEventIndex() == eventIndex) {
						shouldSpawn = false;
						break;
					}
				}
				if (shouldSpawn) {
					LaneNote* newNote = allocNoteFromPool((int32)eventIndex, noteBrushes[evt.laneIndex]);
					newNote->setX(getLaneX(evt.laneIndex));
					column.push_back(newNote);
					noteContainer->addChild(newNote);
				}
			}

			// Assumes events are sorted by time.
			if (currT < evt.pressTime - KEY_DROP_PERIOD) {
				lastSearchedEventIndex[laneIx] = (int32)eventIndex;
				break;
			}
		}
	}

	// Update note positions.
	for (uint32 laneIx = 0; laneIx < LANE_COUNT; ++laneIx) {
		auto& column = laneNoteColumns[laneIx];
		const auto& laneEvents = loadedRecord.getLaneEvents(laneIx);

		for (size_t i = 0; i < column.size(); ++i) {
			LaneNote* note = column[i];

			const LaneKeyEvent& evt = laneEvents[note->getEventIndex()];
			bool isShortNote = evt.isShortNote();

			float pressDistance = (evt.pressTime - currT) / KEY_DROP_PERIOD;
			float releaseDistance = (evt.releaseTime - currT) / KEY_DROP_PERIOD;

			bool canCatch = pressDistance >= -CATCH_RATIO_GOOD;
			if (isShortNote) {
				note->setY(getShortNoteY(1.0f - pressDistance));
			} else {
				float noteY, noteHeight;
				getLongNoteTransform(
					currT, evt.pressTime, evt.releaseTime,
					&noteY, &noteHeight);
				note->setY(noteY);
				note->setSize(NOTE_WIDTH, noteHeight);
			}

			bool deleteShortNote = isShortNote && (canCatch == false || note->getCatched());
			bool deleteLongNote = !isShortNote && (releaseDistance < -CATCH_RATIO_GOOD);
			if (deleteShortNote || deleteLongNote) {
				if (note->getCatched() == false) {
					scoreboardData.nMiss += 1;
					setJudge(currentGameTime, JUDGE_TYPE_MISS);
				}
				noteContainer->removeChild(note);
				column.erase(column.begin() + i);
				returnNoteToPool(note);
				--i;
			}
		}
	}

	// Update catch effects.
	if (bCatchEffectAllValid) {
		for (CatchEffect* catchEffect : laneCatchffects) {
			catchEffect->updateEffect(currT);
		}
	}

	// Update visibility of judge label.
	bool bShowJudge = (currT - judgeTime <= JUDGE_DISPLAY_PERIOD);
	judgeLabel->setVisible(bShowJudge);
	// #todo-rhythm: Label scaling is bugged
	//float judgeScale = glm::mix(1.0f, 1.2f, (currT - judgeTime) / JUDGE_DISPLAY_PERIOD);
	//judgeLabel->setScaleX(judgeScale);
	//judgeLabel->setScaleY(judgeScale);
	vector3 judgeColor = glm::mix(JUDGE_COLOR, JUDGE_COLOR_FADE, (currT - judgeTime) / JUDGE_DISPLAY_PERIOD);
	judgeLabel->setColor(judgeColor);

	// Update score labels.
	wchar_t scoreText[256];
	swprintf_s(scoreText, L"PERFECT : %d", scoreboardData.nPerfect);
	perfectLabel->setText(scoreText);
	swprintf_s(scoreText, L"GOOD    : %d", scoreboardData.nGood);
	goodLabel->setText(scoreText);
	swprintf_s(scoreText, L"MISS    : %d", scoreboardData.nMiss);
	missLabel->setText(scoreText);
}

void World_RhythmGame::setJudge(float currentT, int32 judgeType) {
	judgeTime = currentT;
	if (judgeType == JUDGE_TYPE_PERFECT) {
		judgeLabel->setText(L"PERFECT");
	} else if (judgeType == JUDGE_TYPE_GOOD) {
		judgeLabel->setText(L" GOOD ");
	} else if (judgeType == JUDGE_TYPE_MISS){
		judgeLabel->setText(L" MISS ");
	}
}

LaneNote* World_RhythmGame::allocNoteFromPool(int32 eventIndex, pathos::Brush* brush) {
	LaneNote* note = nullptr;

	if (noteObjectPool.size() > 0) {
		note = noteObjectPool[noteObjectPool.size() - 1];
		noteObjectPool.pop_back();
	} else {
		LOG(LogWarning, "Note pool is empty; creating a new instance");
		note = new LaneNote();
	}

	note->setCatched(false);
	note->setEventIndex(eventIndex);
	note->setBrush(brush);
	note->setSize(NOTE_WIDTH, NOTE_HEIGHT);
	return note;
}

void World_RhythmGame::returnNoteToPool(LaneNote* note) {
	noteObjectPool.push_back(note);
}

void World_RhythmGame::setMusicVolume(float value) {
	if (musicStream != nullptr) {
		musicStream->setVolume(value);
	}
}

void World_RhythmGame::browseMusicList(int32 delta) {
	if (gameState == GameState::BrowseMusic) {
		int32 targetIx = browserWidget->getSelectedIndex() + delta;
		browserWidget->selectItem(targetIx);
	}
}

void World_RhythmGame::onPressLaneKey(int32 laneIndex) {
	if (gameState != GameState::PlaySession) {
		return;
	}

	// Record input
	if (laneKeyPressTimes[laneIndex] >= 0.0f) {
		LOG(LogError, "[OnPress] Previous key press was not handled (lane %d)", laneIndex);
	}
	laneKeyPressTimes[laneIndex] = currentGameTime;

	// Toggle effect
	lanePressEffects[laneIndex]->setVisible(true);

	// Process current play
	const auto& laneEvents = loadedRecord.getLaneEvents(laneIndex);
	bool bAnyCatched = false;

	for (LaneNote* note : laneNoteColumns[laneIndex]) {
		if (note->getCatched()) {
			continue;
		}

		const LaneKeyEvent& evt = laneEvents[note->getEventIndex()];
		bool isShortNote = evt.isShortNote();

		float ratio = std::abs((evt.pressTime - currentGameTime) / KEY_DROP_PERIOD);
		if (ratio <= CATCH_RATIO_PERFECT) {
			if (isShortNote) {
				scoreboardData.nPerfect += 1;
			} else {
				// Long note score is processed in release event.
			}
			bAnyCatched = true;
			note->setCatched(true);
			setJudge(currentGameTime, JUDGE_TYPE_PERFECT);
			if (bCatchEffectAllValid) {
				laneCatchffects[laneIndex]->playEffect(currentGameTime);
			}
		} else if (ratio <= CATCH_RATIO_GOOD) {
			if (isShortNote) {
				scoreboardData.nGood += 1;
			} else {
				// Long note score is processed in release event.
			}
			bAnyCatched = true;
			note->setCatched(true);
			setJudge(currentGameTime, JUDGE_TYPE_GOOD);
			if (bCatchEffectAllValid) {
				laneCatchffects[laneIndex]->playEffect(currentGameTime);
			}
		}

		// Assumes notes are sorted by time.
		if (bAnyCatched) {
			break;
		}
	}
}

void World_RhythmGame::onReleaseLaneKey(int32 laneIndex) {
	if (gameState != GameState::PlaySession) {
		return;
	}

	// Record input
	float noteStartTime = laneKeyPressTimes[laneIndex];
	float noteEndTime = currentGameTime;
	float timeDiff = noteEndTime - noteStartTime;

	if (noteStartTime >= 0.0f && timeDiff > 0.0f) {
		if (timeDiff <= SHORT_NOTE_PERIOD) {
			recordToSave.addShortNoteEvent(laneIndex, noteStartTime);
		} else {
			recordToSave.addLongNoteEvent(laneIndex, noteStartTime, noteEndTime);
		}
	} else {
		LOG(LogError, "[OnRelease] Key recording is bugged (lane %d)", laneIndex);
	}

	laneKeyPressTimes[laneIndex] = -1.0f;

	// Toggle effect
	lanePressEffects[laneIndex]->setVisible(false);

	// Process current play
	const auto& laneEvents = loadedRecord.getLaneEvents(laneIndex);
	for (LaneNote* note : laneNoteColumns[laneIndex]) {
		if (note->getCatched()) {
			const LaneKeyEvent& evt = laneEvents[note->getEventIndex()];
			if (evt.isShortNote()) {
				continue;
			}

			float ratio = std::abs((evt.releaseTime - currentGameTime) / KEY_DROP_PERIOD);
			if (ratio <= CATCH_RATIO_PERFECT) {
				scoreboardData.nPerfect += 1;
				setJudge(currentGameTime, JUDGE_TYPE_PERFECT);
				if (bCatchEffectAllValid) {
					laneCatchffects[laneIndex]->playEffect(currentGameTime);
				}
			} else if (ratio <= CATCH_RATIO_GOOD) {
				scoreboardData.nGood += 1;
				setJudge(currentGameTime, JUDGE_TYPE_GOOD);
				if (bCatchEffectAllValid) {
					laneCatchffects[laneIndex]->playEffect(currentGameTime);
				}
			} else {
				note->setCatched(false);
				// Assumes notes are sorted by time.
				break;
			}
		}
	}
}
