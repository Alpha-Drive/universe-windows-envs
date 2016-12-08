#include "JoystickController.h"
#include <cstdlib>

#include <Windows.h>

#pragma comment(lib, "vJoyInterface")

#include "public.h"
#include "vjoyinterface.h"
#include <stdexcept>
#include "Common.h"
#include <string>
#include <json/json.h>

JoystickController::JoystickController() : lg_()
{
	// Get the driver attributes (Vendor ID, Product ID, Version Number)
	if (!vJoyEnabled())
	{
		BOOST_LOG_SEV(lg_, ls::error) << "Failed Getting vJoy attributes. Is vJoy driver installed?";
		exit(1);
	}

	bool aqcuired = AcquireVJD(kJoyId_);
	auto status  = GetVJDStatus(kJoyId_);
	if( ! aqcuired )
	{
		BOOST_LOG_SEV(lg_, ls::error) << "Could not acquire vjoy. You may need to close other apps using Vjoy like Vjoy feeder.";
		exit(1);
	}
}

JoystickController::~JoystickController()
{
}

bool JoystickController::set_axis_(double value, const UINT vjoy_axis)
{
	return SetAxis(normalize_joystick_input_(value), kJoyId_, vjoy_axis);
}

double JoystickController::normalize_joystick_input_(const double value)
{
	return 32767.0 / 2 * (1 + value);
}

void JoystickController::set(const Json::Value& action)
{
	if(action[0] == "JoystickAxisXEvent")
	{
		this->set_x_axis(action[1].asDouble());
	}
	else if(action[0] == "JoystickAxisYEvent")
	{
		this->set_y_axis(action[1].asDouble());
	}
	else if(action[0] == "JoystickAxisZEvent")
	{
		this->set_z_axis(action[1].asDouble());
	}
	else if(action[0] == "JoystickAxisRxEvent")
	{
		this->set_rx_axis(action[1].asDouble());
	}
	else if(action[0] == "JoystickAxisRyEvent")
	{
		this->set_ry_axis(action[1].asDouble());
	}
	else if(action[0] == "JoystickAxisRzEvent")
	{
		this->set_rz_axis(action[1].asDouble());
	}
	else if(action[0] == "JoystickSlider0Event")
	{
		this->set_slider_0(action[1].asDouble());
	}
	else if(action[0] == "JoystickSlider1Event")
	{
		this->set_slider_1(action[1].asDouble());
	}
}

void JoystickController::set_x_axis(double value)
{
	this->set_axis_(value, HID_USAGE_X);
}

void JoystickController::set_y_axis(double value)
{
	this->set_axis_(value, HID_USAGE_Y);
}

void JoystickController::set_z_axis(double value)
{
	this->set_axis_(value, HID_USAGE_Z);
}

void JoystickController::set_rx_axis(double value)
{
	this->set_axis_(value, HID_USAGE_X);
}

void JoystickController::set_ry_axis(double value)
{
	this->set_axis_(value, HID_USAGE_RY);
}

void JoystickController::set_rz_axis(double value)
{
	this->set_axis_(value, HID_USAGE_RZ);
}

void JoystickController::set_slider_0(double value)
{
	this->set_axis_(value, HID_USAGE_SL0);
}

void JoystickController::set_slider_1(double value)
{
	this->set_axis_(value, HID_USAGE_SL1);
}