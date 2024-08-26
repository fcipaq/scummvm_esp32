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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "osystem.h"
#include "engines/engine.h"

namespace _Esp32 {

static Common::Mutex *_eventMutex;

volatile SemaphoreHandle_t _lockEventThread = xSemaphoreCreateBinary();
volatile SemaphoreHandle_t _lock_get_gamepad = xSemaphoreCreateBinary();

static esp_gamepad_state_t _gamepad_state;
static esp_gamepad_state_t _gamepad_state_last;

uint8_t debounce[KEY_MAX];

volatile int _cursorX = 160;
volatile int _cursorY = 100;
volatile int _lastCursorX = 0;
volatile int _lastCursorY = 0;

uint16_t OSystem_Esp32::getCursorPosX() {
  return _cursorX;
}

uint16_t OSystem_Esp32::getCursorPosY() {
  return _cursorY;
}

void OSystem_Esp32::setCursorPosX(uint16_t posx) {
  _cursorX = posx;
}

void OSystem_Esp32::setCursorPosY(uint16_t posy) {
  _cursorY = posy;
}

void OSystem_Esp32::get_gamepad_state(esp_gamepad_state_t* out_state) {
  if (!_isInputInit) abort();

  xSemaphoreTake(_lock_get_gamepad, portMAX_DELAY);

  for (int i = 0; i < KEY_MAX; i++)
    out_state->value[i] = _gamepad_state.value[i];

  xSemaphoreGive(_lock_get_gamepad);
}

void OSystem_Esp32::pushEventQueue(Common::Queue<Common::Event> *queue, Common::Event &event) {
	Common::StackLock lock(*_eventMutex);
	queue->push(event);
}

void OSystem_Esp32::report_keystate_to_engine() {
  int ax = 0;
  int ay = 0;

  int saveload = 0;

  Common::Event event;

  if (!(_gamepad_state_last.value[KEY_1]) && (_gamepad_state.value[KEY_1])) {
    event.type = Common::EVENT_KEYDOWN;
    event.kbd.keycode = Common::KEYCODE_ESCAPE;
    event.kbd.ascii = Common::ASCII_ESCAPE;
    event.kbd.flags = 0;
    pushEventQueue(&_eventQueue, event);
  } else if ((_gamepad_state_last.value[KEY_1]) && (!_gamepad_state.value[KEY_1])) {
    event.type = Common::EVENT_KEYUP;
    event.kbd.keycode = Common::KEYCODE_ESCAPE;
    event.kbd.ascii = Common::ASCII_ESCAPE;
    event.kbd.flags = 0;
    pushEventQueue(&_eventQueue, event);
  }

  if (!(_gamepad_state_last.value[KEY_2]) && (_gamepad_state.value[KEY_2])) {
    event.type = Common::EVENT_LBUTTONDOWN;
    event.mouse.x = _cursorX;
    event.mouse.y = _cursorY;
    pushEventQueue(&_eventQueue, event);
  } else if ((_gamepad_state_last.value[KEY_2]) && (!_gamepad_state.value[KEY_2])) {
    event.type = Common::EVENT_LBUTTONUP;
    event.mouse.x = _cursorX;
    event.mouse.y = _cursorY;
    pushEventQueue(&_eventQueue, event);
  }

  if (!(_gamepad_state_last.value[KEY_3]) && (_gamepad_state.value[KEY_3])) {
    event.type = Common::EVENT_RBUTTONDOWN;
    event.mouse.x = _cursorX;
    event.mouse.y = _cursorY;
    pushEventQueue(&_eventQueue, event);
  } else if ((_gamepad_state_last.value[KEY_3]) && (!_gamepad_state.value[KEY_3])) {
    event.type = Common::EVENT_RBUTTONUP;
    event.mouse.x = _cursorX;
    event.mouse.y = _cursorY;
    pushEventQueue(&_eventQueue, event);
  }  
 
  if ( _gamepad_state.value[KEY_1] &&
       _gamepad_state.value[KEY_2] &&
       _gamepad_state.value[KEY_3] ) {
    saveload = 1;
    event.type = Common::EVENT_KEYDOWN;
    event.kbd.keycode = Common::KEYCODE_1;
    event.kbd.ascii = '1';
    if (_gamepad_state.value[KEY_UP] && (!_gamepad_state_last.value[KEY_UP])) {
      event.kbd.flags = Common::KBD_ALT;
      printf("Gamestate saved.\n");
      pushEventQueue(&_eventQueue, event);
    } else if (_gamepad_state.value[KEY_DOWN] && (!_gamepad_state_last.value[KEY_DOWN])) {
      event.kbd.flags = Common::KBD_CTRL;
      printf("Gamestate loaded.\n");
      pushEventQueue(&_eventQueue, event);
    } else if (_gamepad_state.value[KEY_LEFT] && (!_gamepad_state_last.value[KEY_LEFT])) {
      _brightness -= 5;
      if (_brightness < 5)
        _brightness = 5;
      backlight_set(_brightness); 
    } else if (_gamepad_state.value[KEY_RIGHT] && (!_gamepad_state_last.value[KEY_RIGHT])) {
      _brightness += 5;
      if (_brightness > 100)
        _brightness = 100;
      backlight_set(_brightness); 
    }
  } else if (saveload) {
    saveload = 0;
    event.type = Common::EVENT_KEYUP;
    event.kbd.keycode = Common::KEYCODE_1;
    event.kbd.ascii = '1';
    event.kbd.flags = Common::KBD_CTRL;
    event.kbd.flags = Common::KBD_ALT;   
    pushEventQueue(&_eventQueue, event);
  }
  
    
#if 0

  if (!(_gamepad_state_last & KEY_MENU) && (keystate & KEY_MENU)) {
    event.type = Common::EVENT_KEYDOWN;
    event.kbd.keycode = Common::KEYCODE_1;
    event.kbd.ascii = '1';
    if (keystate & KEY_SELECT) {
      event.kbd.flags = Common::KBD_CTRL;
    } else {
      event.kbd.flags = Common::KBD_ALT;
    }
    //pushEventQueue(&_eventQueue, event);
    //printf("receiveKeyState(4):Send keydown / ALT+1\n");
    pushEventQueue(&_eventQueue, event);
  }

  if ((_gamepad_state_last & KEY_MENU) && !(keystate & KEY_MENU)) {
    event.type = Common::EVENT_KEYUP;
    event.kbd.keycode = Common::KEYCODE_1;
    event.kbd.ascii = '1';
    if (keystate & KEY_SELECT) {
      event.kbd.flags = Common::KBD_CTRL;
    } else {
      event.kbd.flags = Common::KBD_ALT;
    }
    //printf("receiveKeyState(5):Send keyup / ALT+1\n");
    //pushEventQueue(&_eventQueue, event);
    pushEventQueue(&_eventQueue, event);
  }

  if (!(_gamepad_state_last & KEY_VOLUME) && (keystate & KEY_VOLUME)) {
    event.type = Common::EVENT_KEYDOWN;
    event.kbd.keycode = Common::KEYCODE_1;
    event.kbd.ascii = '1';
    /*if(keystate&KEY_SELECT){
				event.kbd.flags = Common::KBD_CTRL;
			}else{
				event.kbd.flags = Common::KBD_ALT;
			}*/
    //pushEventQueue(&_eventQueue, event);
    //printf("receiveKeyState(4):Send keydown / ALT+1\n");
    pushEventQueue(&_eventQueue, event);
  }

  if ((_gamepad_state_last & KEY_VOLUME) && !(keystate & KEY_VOLUME)) {
    event.type = Common::EVENT_KEYUP;
    event.kbd.keycode = Common::KEYCODE_1;
    event.kbd.ascii = '1';
    /*if(keystate&KEY_SELECT){
				event.kbd.flags = Common::KBD_CTRL;
			}else{
				event.kbd.flags = Common::KBD_ALT;
			}*/
    //printf("receiveKeyState(5):Send keyup / ALT+1\n");
    //pushEventQueue(&_eventQueue, event);
    pushEventQueue(&_eventQueue, event);
  }

  if (!(_gamepad_state_last & KEY_A) && (keystate & KEY_A)) {
    event.type = Common::EVENT_LBUTTONDOWN;
    event.mouse.x = _cursorX;
    event.mouse.y = _cursorY;
    pushEventQueue(&_eventQueue, event);
  }

  if ((_gamepad_state_last & KEY_A) && !(keystate & KEY_A)) {
    event.type = Common::EVENT_LBUTTONUP;
    event.mouse.x = _cursorX;
    event.mouse.y = _cursorY;
    pushEventQueue(&_eventQueue, event);
  }

  if (!(_gamepad_state_last & KEY_B) && (keystate & KEY_B)) {
    event.type = Common::EVENT_RBUTTONDOWN;
    event.mouse.x = _cursorX;
    event.mouse.y = _cursorY;
    pushEventQueue(&_eventQueue, event);
  }

  if ((_gamepad_state_last & KEY_B) && !(keystate & KEY_B)) {
    event.type = Common::EVENT_RBUTTONUP;
    event.mouse.x = _cursorX;
    event.mouse.y = _cursorY;
    pushEventQueue(&_eventQueue, event);
  }
#endif

  if (!(_gamepad_state.value[KEY_RIGHT] | _gamepad_state.value[KEY_LEFT])) {
    ax = 0;
  } else {
    if (_gamepad_state.value[KEY_RIGHT]) {
      ax++;
    } else if (_gamepad_state.value[KEY_LEFT]) {
      ax--;
    }
  }

  _cursorX += ax;
  if (_cursorX < 0) _cursorX = 0;
  if (_cursorX > 319) _cursorX = 319;

  if (!(_gamepad_state.value[KEY_UP] | _gamepad_state.value[KEY_DOWN])) {
    ay = 0;
  } else {
    if (_gamepad_state.value[KEY_UP]) {
      ay--;
    } else if (_gamepad_state.value[KEY_DOWN]) {
      ay++;
    }
  }

  _cursorY += ay;
  if (_cursorY < 0) _cursorY = 0;
  if (_cursorY > 199) _cursorY = 199;
    
}

static void eventThreadFunc(void *arg) {
  OSystem_Esp32* osys = (OSystem_Esp32*) arg;  

  // Initialize state
  for (int i = 0; i < KEY_MAX; ++i) {
    debounce[i] = 0xff;
  }

  while (1) {

    // Shift current values
    for (int i = 0; i < KEY_MAX; ++i) {
      debounce[i] <<= 1;
    }

    xSemaphoreTake(_lockEventThread, portMAX_DELAY);
    // Read hardware
    esp_gamepad_state_t state = osys->hw_input_read_raw();

    // Debounce
    for (int i = 0; i < KEY_MAX; ++i) {
      debounce[i] |= state.value[i] ? 1 : 0;
      uint8_t val = debounce[i] & 0x03;  //0x0f;
      switch (val) {
        case 0x00:
          _gamepad_state.value[i] = 0;
          break;

        case 0x03:  //0x0f:
          _gamepad_state.value[i] = 1;
          break;

        default:
          // ignore
          break;
      }

    }

    osys->report_keystate_to_engine();

    // fcipaq
    for (int i = 0; i < KEY_MAX; i++)
      _gamepad_state_last.value[i] = _gamepad_state.value[i];

    xSemaphoreGive(_lockEventThread);

    // delay
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

}

/*
static void timerThreadFunc(void *arg) {
	OSystem_Esp32 *osys = (OSystem_Esp32 *)arg;
	DefaultTimerManager *tm = (DefaultTimerManager *)osys->getTimerManager();
	while (!osys->exiting) {
		g_system->delayMillis(10);
		tm->handler();
	}
}
*/

void OSystem_Esp32::initEvents() {
  _eventMutex = new Common::Mutex();
  
  assert(_lockEventThread);
  xSemaphoreGive(_lockEventThread);
  assert(_lock_get_gamepad);
  xSemaphoreGive(_lock_get_gamepad);
  

  // Start background polling
  xTaskCreatePinnedToCore((TaskFunction_t) &eventThreadFunc, "eventThreadFunc", 1024 * 2, this, 5, NULL, 0);
}

void OSystem_Esp32::destroyEvents() {
/*
	threadJoin(_timerThread, U64_MAX);
	threadFree(_timerThread);

	threadJoin(_eventThread, U64_MAX);
	threadFree(_eventThread);
	delete _eventMutex;
*/
}

bool OSystem_Esp32::pollEvent(Common::Event &event) {
  Common::StackLock lock(*_eventMutex);

  if ((_lastCursorX != _cursorX) || (_lastCursorY != _cursorY)) {
    Common::Event event;
    event.type = Common::EVENT_MOUSEMOVE;
    event.mouse.x = _cursorX;
    event.mouse.y = _cursorY;
    _lastCursorX = _cursorX;
    _lastCursorY = _cursorY;
    _eventQueue.push(event);
  }

  if (_eventQueue.empty())
    return false;

  event = _eventQueue.pop();
  return true;
}

} // namespace _Esp32
