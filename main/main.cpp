/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
 
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include <string.h>

#include "engines/engine.h"
#include "engines/metaengine.h"
#include "base/commandLine.h"
#include "base/plugins.h"
#include "base/version.h"

#include "common/language.h"
#include "scumm/detection.h"
#include "scumm/detection_tables.h"
#include "scumm/scumm.h"
#include "scumm/scummmetaengine.h"
#include "scumm/scumm_v5.h"

#include "cine/cinemetaengine.h"

#include "common/config-manager.h"

#include "backends/platform/esp32/osystem.h"

#include "image_splash.h"

// freeRTOS calls app_main from C 
extern "C" {
  extern void app_main();
}

//Common::Language language = Common::GR_GRE;
char base[256];
char dir[256];
char savedir[256];

void select_game() {
  // Show Folder Selection
  char const *basedir = "/sd/roms/scummvm";

  int selectedFolder = 0;

  Common::FSNode basedirNode = Common::FSNode(basedir);
  Common::FSList directories;
  basedirNode.getChildren(directories, Common::FSNode::kListDirectoriesOnly);

  // Read Gamepad once
  _Esp32::esp_gamepad_state_t lastJoysticState;
  _Esp32::esp_gamepad_state_t joysticState;

  ((_Esp32::OSystem_Esp32*) g_system)->get_gamepad_state(&lastJoysticState);

  uint16_t c_black = 7;

  bool keypressed = false;
  bool startPressed = false;

  uint8_t* dst = (uint8_t*) ((_Esp32::OSystem_Esp32*) g_system)->getScreenBuffer();

  while (!startPressed) {
    Common::String gameName = Common::String("> ") + directories[selectedFolder].getName() + Common::String("                    ");

    for (int i = 0; i < 320 * 240; i++)
      dst[i] = image_splash[i];

    ((_Esp32::OSystem_Esp32*) g_system)->draw_string(30, 185, "Select Game:", c_black);
    ((_Esp32::OSystem_Esp32*) g_system)->draw_string(30, 200, gameName.c_str(), c_black);
    ((_Esp32::OSystem_Esp32*) g_system)->draw_string(30, 215, "Press a button to play!", c_black);

    ((_Esp32::OSystem_Esp32*) g_system)->display_send_buf_raw((void*) ((_Esp32::OSystem_Esp32*) g_system)->getScreenBuffer(),
    							              320,
  							              240,
  							              (uint16_t*) image_splash_palette);

    keypressed = false;
    while (!keypressed) {
          ((_Esp32::OSystem_Esp32*) g_system)->get_gamepad_state(&joysticState);

      if (joysticState.value[_Esp32::KEY_RIGHT] && !lastJoysticState.value[_Esp32::KEY_RIGHT]) {
        if (selectedFolder < directories.size() - 1) {
          selectedFolder++;
        }
        keypressed = true;
      }

      if (joysticState.value[_Esp32::KEY_LEFT] && !lastJoysticState.value[_Esp32::KEY_LEFT]) {
        if (selectedFolder > 0) {
          selectedFolder--;
        }
        keypressed = true;
      }

      if ((joysticState.value[_Esp32::KEY_1] && !lastJoysticState.value[_Esp32::KEY_1]) ||
          (joysticState.value[_Esp32::KEY_2] && !lastJoysticState.value[_Esp32::KEY_2]) ||
          (joysticState.value[_Esp32::KEY_3] && !lastJoysticState.value[_Esp32::KEY_3])) {
        startPressed = true;
        keypressed = true;
      }
      lastJoysticState = joysticState;
    }
  }

  for (int i = 0; i < 320 * 240; i++)
    dst[i] = 0;

  sprintf(base, "%s", directories[selectedFolder].getName().c_str());
  sprintf(dir, "%s", directories[selectedFolder].getPath().c_str());
  sprintf(savedir, "%s/saves", directories[selectedFolder].getPath().c_str());

  //printf("Selected path: %s\n", dir);
  //printf("Base: %s\n", base);

  ((_Esp32::OSystem_Esp32 *) g_system)->setSaveDir(savedir);
}

void app_main(void) {
  // Prepare system
  g_system = new _Esp32::OSystem_Esp32();
  assert(g_system);  // fcipaq: esp_system implements more functions than g_system - hence the hussle
  
  g_system->initBackend();

  Base::registerDefaults();

  // Load the config file
  ConfMan.loadDefaultConfigFile();
  ConfMan.setBool("subtitles", false);
  ConfMan.setInt("talkspeed", 9);
  ConfMan.registerDefault("fullscreen", true);
  ConfMan.registerDefault("aspect_ratio", true);

  PluginManager::instance().init();
  PluginManager::instance().loadAllPlugins();  // load plugins for cached plugin manager
  
  select_game();  

  // Make folder for savegames
  //mkdir(savedir, ACCESSPERMS);

  Common::Language language = Common::UNK_LANG;
  Common::Platform platform = Common::kPlatformUnknown;

  // Set the game path.
  ConfMan.addGameDomain(base);

  ConfMan.set("path", dir, base);

  // Set the game language.
  ConfMan.set("language", Common::getLanguageCode(language), base);

  // Set the game platform.
  ConfMan.set("platform", Common::getPlatformCode(platform), base);

  // Set the target.
  ConfMan.setActiveDomain(base);

  Common::FSNode dirNode = Common::FSNode(dir);
  Common::FSList files;
  assert(dirNode.getChildren(files, Common::FSNode::kListAll));

  Cine::CineMetaEngine cmEngine;
  Scumm::ScummMetaEngine smEngine;

  MetaEngine *engines[] = { &smEngine, &cmEngine };

  MetaEngine *mEngine;  // = smEngine;
  DetectedGames games;  // = mEngine.detectGames(files);

  //Common::List<Scumm::DetectorResult> results;

  for (int i = 0; i < 2; i++) {
    mEngine = engines[i];
    games = mEngine->detectGames(files);
    if (!games.empty()) break;
  }

  if (games.empty()) {
    printf("No Engine found to run this game!\n");
    abort();
  }

  printf("Gameid: %s\n", games[0].gameId.c_str());
  ConfMan.set("gameid", games[0].gameId, base);

  Engine *engine;
  mEngine->createInstance(g_system, &engine);

  // Inform backend that the engine is about to be run
  g_system->engineInit();

  // Run the engine
  Common::Error result = engine->run();
  ((_Esp32::OSystem_Esp32*) g_system)->draw_string(30, 50, "Something went wrong...", 0xaffe);
}

