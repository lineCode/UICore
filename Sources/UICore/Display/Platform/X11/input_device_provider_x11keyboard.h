/*
**  UICore
**  Copyright (c) 1997-2015 The UICore Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries UICore may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Magnus Norddahl
**    Mark Page
*/

#pragma once

#include "UICore/Display/Window/input_device.h"
#include "UICore/Display/Window/input_device_provider.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <map>

namespace uicore
{
	class X11Window;

	class InputDeviceProvider_X11Keyboard : public InputDeviceProvider
	{
	public:
		InputDeviceProvider_X11Keyboard(X11Window *window);
		~InputDeviceProvider_X11Keyboard();

		InputDevice::Type type() const override { return InputDevice::keyboard; }
		bool keycode(int keycode) const override;
		std::string key_name(int id) const override;
		std::string name() const override;
		std::string device_name() const override;
		int button_count() const override { return -1; }

		void get_keyboard_modifiers(bool &key_shift, bool &key_alt, bool &key_ctrl) const;

		void received_keyboard_input(std::shared_ptr<InputDevice> &keyboard, XKeyEvent &event);

	private:
		void on_dispose() override;

		X11Window *window;

		std::map<int, int> repeat_count;
		bool ctrl_down, shift_down, alt_down;

		std::map<KeySym, int> current_keys_down;
	};
}
