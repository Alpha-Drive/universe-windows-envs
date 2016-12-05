#ifndef JOYSTICK_CONTROLLER_H_
#define JOYSTICK_CONTROLLER_H_

#include <json/json.h>
#include "Common.h"

class JoystickController
{
public:
	JoystickController();
	~JoystickController();
	void set(const Json::Value& action);
	void set_x_axis(double value);
	void set_y_axis(double value);
	void set_z_axis(double value);
	void set_rx_axis(double value);
	void set_ry_axis(double value);
	void set_rz_axis(double value);
	void set_slider_0(double value);
	void set_slider_1(double value);
private:
	const int kJoyId_ = 1; // assume only joystick installed on system
	static double normalize_joystick_input_(const double value);
	bool set_axis_(const double value, const unsigned Axis);
	boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> lg_;
};

#endif // !JOYSTICK_CONTROLLER_H_