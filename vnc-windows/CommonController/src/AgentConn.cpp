#include <curl/curl.h>
#include "AgentConn.h"
#include <chrono>
#include "Common.h"
#include <mutex>
#include <allocators>
#include <thread>
#include <Utils.h>
#include <boost/signals2.hpp>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

bool websocket_send(server* ws_server, websocketpp::connection_hdl& cxn, std::string const& msg)
{
	try {
		ws_server->send(cxn, msg, websocketpp::frame::opcode::text);
		return true;
	} catch (const websocketpp::lib::error_code& e) {
		BOOST_LOG_TRIVIAL(error) << "Echo failed because: " << e
			<< "(" << e.message() << ")";
		return false;
	}
}

void AgentConn::run_server_thread()
{
	BOOST_LOG_TRIVIAL(info) << "Starting websocket server on port " << std::to_string(port_).c_str();
	websocket_server_.set_open_handler(bind(&AgentConn::on_websocket_open_, this, ::_1));
	websocket_server_.set_close_handler(bind(&AgentConn::on_websocket_close_, this, ::_1));
	websocket_server_.set_message_handler(bind(&AgentConn::on_websocket_msg_, this, ::_1, ::_2));
	websocket_server_.init_asio();
	websocket_server_.listen(port_);
	websocket_server_.start_accept();
	websocket_server_.run();
	BOOST_LOG_TRIVIAL(info) << "Websocket server stopped";
}

AgentConn::AgentConn(int port)
{
	port_ = port;
	server_thread_.reset(new std::thread(&AgentConn::run_server_thread, this));
	server_thread_->detach();
}

AgentConn::~AgentConn()
{
	BOOST_LOG_TRIVIAL(info) << "Agent conn shutting down";
	websocket_server_.stop();
    websocketpp::lib::error_code ec;
    websocket_server_.stop_listening(ec);
	BOOST_LOG_TRIVIAL(info) << "Agent conn shutdown successfully";
}

void AgentConn::send_env_describe(std::string const& env_id, std::string const& env_state, int episode_id, int fps, std::string const& metadata)
{
	Json::Value body;
	body["env_id"] = env_id;
	body["env_state"] = env_state;
	body["metadata"] = metadata;
	body["fps"] = fps;
	send_json_("v0.env.describe", body, episode_id);
}

void AgentConn::send_reply_control_ping(Json::Value const& request)
{
	Json::Value body = Json::Value::null;
	const long long parent_message_id = request["headers"]["message_id"].asInt64();
	send_json_("v0.reply.control.ping", body, -1, parent_message_id);
}

void AgentConn::send_reset_reply(Json::Value const& request, int episode_id)
{
	Json::Value body = Json::Value::null;
	const long long parent_message_id = request["headers"]["message_id"].asInt64();
	send_json_("v0.reply.env.reset", body, episode_id, parent_message_id);
}

void AgentConn::send_reward(const double reward, int episode_id, const bool done, Json::Value info)
{
	Json::Value body;
	body["reward"] = reward;
	body["done"] = done;
	body["info"] = info;
	send_json_("v0.env.reward", body, episode_id);
}

void AgentConn::populate_sent_at_in_headers_(Json::Value& headers)
{
	time_t seconds_past_epoch = time(0);

	using namespace std::chrono;
	microseconds micros = duration_cast<microseconds>(
		system_clock::now().time_since_epoch()
	);
	long long millis = micros.count();
	double seconds_since_epoch = micros.count() / 1000000.0;
	headers["sent_at"] = seconds_since_epoch;
}

boost::signals2::connection AgentConn::on_reset(const ResetSignal::slot_type& subscriber)
{
	return reset_signal_.connect(subscriber);
}

boost::signals2::connection AgentConn::on_action(const ActionSignal::slot_type& subscriber)
{
	return action_signal_.connect(subscriber);
}

boost::signals2::connection AgentConn::on_no_clients(const NoClientsSignal::slot_type& subscriber)
{
	return no_clients_signal_.connect(subscriber);
}

Json::Value AgentConn::get_headers_(const long long parent_message_id, int episode_id)
{
	Json::Value headers;
	populate_sent_at_in_headers_(headers);

	if(parent_message_id != -1)
	{
		headers["parent_message_id"] = parent_message_id;
	}
	if(episode_id != -1)
	{
		headers["episode_id"] = std::to_string(episode_id);
	}
	return headers;
}

void AgentConn::on_websocket_open_(websocketpp::connection_hdl websocket_cxn) {
	std::lock_guard<std::mutex> guard(m_connection_lock);
	BOOST_LOG_TRIVIAL(info) << "Web socket connection established";
	if(m_connections.size() > 0 && m_connections.count(websocket_cxn) == 0)
	{
		// Multiple client connections
		BOOST_LOG_TRIVIAL(warning) << (m_connections.size() + 1) << " client connections -- should be only one -- sending to most recent connection only.";
	}
	m_connections.insert(websocket_cxn);
	most_recent_websocket_cxn_ = websocket_cxn;
}

void AgentConn::on_websocket_close_(websocketpp::connection_hdl websocket_cxn) {
	std::lock_guard<std::mutex> guard(m_connection_lock);
	m_connections.erase(websocket_cxn);
	BOOST_LOG_TRIVIAL(info) << "Web socket connection closed";
}

void AgentConn::on_websocket_msg_(websocketpp::connection_hdl websocket_cxn, message_ptr msg) {
	Json::Value request = json_loads(msg->get_payload());
//	std::string pretty = json_dumps(request);
//	BOOST_LOG_TRIVIAL(debug) << "Received request:" << std::endl << request);
	if(request == Json::nullValue)
	{
		BOOST_LOG_TRIVIAL(warning) << "Skipping request with malformed JSON";
		return;
	}
	else if(request["method"] == "v0.control.ping")
	{
		BOOST_LOG_TRIVIAL(info) << "Received rpc control ping";
		send_reply_control_ping(request);
	}
	else if(request["method"] == "v0.env.reset")
	{
		BOOST_LOG_TRIVIAL(warning) <<  "Received reset request from agent";
		reset_signal_(request);
	}
	else if(request["method"] == "v0.agent.action")
	{
		BOOST_LOG_TRIVIAL(info) << "Received action from agent";
		action_signal_(request);
	}
}

void AgentConn::send_json_(std::string const& method, Json::Value const& body, int episode_id, const long long parent_message_id)
{
	Json::Value payload;
	payload["method"] = method;
	payload["body"] = body;
	payload["headers"] = get_headers_(parent_message_id, episode_id);
	std::string msg_string = json_dumps(payload);
	if(m_connections.size() < 1)
	{
		BOOST_LOG_TRIVIAL(info) << "No clients connected -- skipping message";
		no_clients_signal_();
		return;
	}
	websocket_send(&websocket_server_, most_recent_websocket_cxn_, msg_string);
}
