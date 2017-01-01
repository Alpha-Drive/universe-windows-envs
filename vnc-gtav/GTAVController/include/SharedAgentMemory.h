#ifndef GTAV_SHARED_AGENT_MEMORY_H_
#define GTAV_SHARED_AGENT_MEMORY_H_


#define AGENT_SHARED_MEMORY TEXT("Local\\GTAVAgent")

struct AgentTime
{
	int year;
	int month;
	int day_of_month;
	int hour;
	int minute;
	int second;
	int ms_per_game_min;
};

struct Vector
{
	double x;
	double y;
	double z;
};

struct SharedAgentMemory
{
	double reward;

	// N.B. ScriptHook driven actions not currently implemented. vJoy used instead.
	double action_throttle; //[0,1]
	double action_brake; //[0,1]
	double action_steer; //[-1,1]. -1 is left, 1 is right

	double x_coord;
	double y_coord;
	double z_coord;
	double dest_x;
	double dest_y;
	double dest_z;
	bool on_road;
	bool should_reset_agent;
	double heading;
	double speed;
	double spin;
	bool should_game_drive;
	bool is_game_driving;
	int script_hook_loadtime;
	bool use_custom_camera;
	double desired_cam_x_offset;
	double desired_cam_y_offset;
	double desired_cam_z_offset;
	double desired_cam_fov;
	double center_of_lane_reward;
	bool is_colliding;
	unsigned long last_material_collided_with;
	int last_collision_time;
	double distance_from_destination;
	Vector forward_vector;
	AgentTime time;
	const char * scenario_name;
};

#endif // !GTAV_SHARED_AGENT_MEMORY_H_
