/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
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
 
#include "osystem.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/events/default/default-events.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace _Esp32 {

	OSystem_Esp32::OSystem_Esp32():
	_brightness(30),
	_cursorPaletteEnabled(false)
	{
	}
	
	
	OSystem_Esp32::~OSystem_Esp32(){}
	
	void OSystem_Esp32::initBackend() {
                _fsFactory = new POSIXFilesystemFactory();

		_timerManager = new DefaultTimerManager();
		gettimeofday(&_startTime, NULL);

		//Posix::assureDirectoryExists("/3ds/scummvm/saves/");		

		initDisplay();
		initGraphics();
		initAudio(20000);
		initInput();
		initEvents();
		if (!initSDcard())
		  assert(0);
		EventsBaseBackend::initBackend();
	}
	

	uint32 OSystem_Esp32::getMillis(bool skipRecord) {
		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);
		return (uint32)(((currentTime.tv_sec - _startTime.tv_sec) * 1000) +
						((currentTime.tv_usec - _startTime.tv_usec) / 1000));// - _timeSuspended;
	}
	
	void OSystem_Esp32::setSaveDir(char* dir){
		savedir = dir;
		_savefileManager = new DefaultSaveFileManager(savedir);
	}

	
	void OSystem_Esp32::delayMillis(uint msecs) {
		vTaskDelay( msecs / portTICK_PERIOD_MS );
		//usleep(msecs * 1000);
		// ToDo: Use ets_delay_us() (defined in rom/ets_sys.h) for small amouts / busy-wait for a correct number of microseconds...

	}
	
	void OSystem_Esp32::getTimeAndDate(TimeDate& td) const {
		time_t curTime = time(0);
		struct tm t = *localtime(&curTime);
		td.tm_sec = t.tm_sec;
		td.tm_min = t.tm_min;
		td.tm_hour = t.tm_hour;
		td.tm_mday = t.tm_mday;
		td.tm_mon = t.tm_mon;
		td.tm_year = t.tm_year;
		td.tm_wday = t.tm_wday;
	}
	
	OpaqueMutex* OSystem_Esp32::createMutex() {
	  SemaphoreHandle_t mutex = xSemaphoreCreateRecursiveMutex();
	  assert(mutex);
	  return (OSystem::MutexRef) mutex;
	}

	void OSystem_Esp32::lockMutex(OSystem::MutexRef mutex) {
	  if (xSemaphoreTakeRecursive( (SemaphoreHandle_t) mutex, portMAX_DELAY ) != pdTRUE) {
	    abort();
	  }
	}

	void OSystem_Esp32::unlockMutex(OSystem::MutexRef mutex ) {
	  xSemaphoreGiveRecursive( (SemaphoreHandle_t) mutex );
	}

	void OSystem_Esp32::deleteMutex(OSystem::MutexRef mutex ) {
	  vSemaphoreDelete( (SemaphoreHandle_t) mutex );
	}
	
	void OSystem_Esp32::quit() {}

	void OSystem_Esp32::logMessage(LogMessageType::Type, const char*) {}

	void logMessage(LogMessageType::Type type, const char *message) {}

	void OSystem_Esp32::resetGraphicsScale(){}

	void OSystem_Esp32::setFocusRectangle(const Common::Rect &rect){}

	void OSystem_Esp32::clearFocusRectangle(){}

	int16 OSystem_Esp32::getOverlayHeight() {return 0;}

	int16 OSystem_Esp32::getOverlayWidth() {return 0;}

	Common::String OSystem_Esp32::getSystemLanguage() const { return "en_US";}

	void OSystem_Esp32::updateConfig() {
		/*
		if (_gameScreen.getPixels()) {
			updateSize();
			warpMouse(_cursorX, _cursorY);
		}*/
	}

	void OSystem_Esp32::flushGameScreen(){
		//printf("OSystem_Esp32::flushGameScreen(1)\n");
	}
	
	void OSystem_Esp32::flushCursor(){}

	
} // namespace _Esp32

