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

AgentConn::AgentConn(int port, boost::log::sources::severity_logger_mt<ls::severity_level> lg)
{
	port_ = port;
	lg_ = lg;
	server_thread_.reset(new std::thread(&AgentConn::run_server_thread, this));
	server_thread_->detach();
}

AgentConn::~AgentConn()
{
	BOOST_LOG_SEV(lg_, ls::info) << "Agent conn shutting down";
	websocket_server_.stop();
	websocketpp::lib::error_code ec;
	websocket_server_.stop_listening(ec);
	BOOST_LOG_SEV(lg_, ls::info) << "Agent conn shutdown successfully";
}

bool websocketSend(server* ws_server, websocketpp::connection_hdl& cxn, std::string const& msg, boost::log::sources::severity_logger_mt<ls::severity_level> lg)
{
	try {
		ws_server->send(cxn, msg, websocketpp::frame::opcode::text);
		return true;
	}
	catch (const websocketpp::lib::error_code& e) {
		BOOST_LOG_SEV(lg, ls::error) << "Send failed because: " << e
			<< "(" << e.message() << ")";
		return false;
	}
	catch (...)
	{
		BOOST_LOG_SEV(lg, ls::error) << "Send failed";
		return false;
	}
}


void AgentConn::run_server_thread()
{
	BOOST_LOG_SEV(lg_, ls::info) << "Starting websocket server on port " << std::to_string(port_).c_str();
	websocket_server_.set_open_handler(bind(&AgentConn::on_websocket_open_, this, ::_1));
	websocket_server_.set_close_handler(bind(&AgentConn::on_websocket_close_, this, ::_1));
	websocket_server_.set_message_handler(bind(&AgentConn::on_websocket_msg_, this, ::_1, ::_2));
#ifndef PROJ_DEBUG
	websocket_server_.clear_access_channels(websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload);
#endif
	websocket_server_.init_asio();
	websocket_server_.listen(port_);
	websocket_server_.start_accept();
	websocket_server_.run();
	BOOST_LOG_SEV(lg_, ls::info) << "Websocket server stopped";
}

void AgentConn::send_env_describe(std::string const& env_id, std::string const& env_state, int episode_id, int fps, std::string const& metadata)
{
	Json::Value body;
	body["env_id"] = env_id;
	body["env_state"] = env_state;
	body["metadata"] = metadata;
	body["fps"] = fps;
	sendJson_("v0.env.describe", body, episode_id);
}

void AgentConn::send_reply_control_ping(Json::Value const& request)
{
	Json::Value body = Json::Value::null;
	const long long parent_message_id = request["headers"]["message_id"].asInt64();
	sendJson_("v0.reply.control.ping", body, -1, parent_message_id);
}

void AgentConn::send_reset_reply(Json::Value const& request, int episode_id)
{
	Json::Value body = Json::Value::null;
	const long long parent_message_id = request["headers"]["message_id"].asInt64();
	sendJson_("v0.reply.env.reset", body, episode_id, parent_message_id);
}

void AgentConn::sendReward(const double reward, int episode_id, const bool done, Json::Value info)
{
	Json::Value body;
	body["reward"] = reward;
	body["done"] = done;
	body["info"] = info;
	sendJson_("v0.env.reward", body, episode_id);
}

bool AgentConn::client_is_connected()
{
	std::lock_guard<std::mutex> guard(m_connection_lock_);
	return m_connections_.size() > 0;
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
	std::lock_guard<std::mutex> guard(m_connection_lock_);
	BOOST_LOG_SEV(lg_, ls::info) << "Web socket connection established";
	if(m_connections_.size() > 0 && m_connections_.count(websocket_cxn) == 0)
	{
		// Multiple client connections
		BOOST_LOG_SEV(lg_, ls::warning) << (m_connections_.size() + 1) << " client connections -- should be only one -- sending to most recent connection only.";
	}
	m_connections_.insert(websocket_cxn);
	most_recent_websocket_cxn_ = websocket_cxn;
}

void AgentConn::on_websocket_close_(websocketpp::connection_hdl websocket_cxn) {
	std::lock_guard<std::mutex> guard(m_connection_lock_);
	m_connections_.erase(websocket_cxn);
	BOOST_LOG_SEV(lg_, ls::info) << "Web socket connection closed";
}

void AgentConn::on_websocket_msg_(websocketpp::connection_hdl websocket_cxn, message_ptr msg) {
	Json::Value request = json_loads(msg->get_payload());
	if(request == Json::nullValue)
	{
		BOOST_LOG_SEV(lg_, ls::warning) << "Skipping request with malformed JSON";
		return;
	}
	else if(request["method"] == "v0.control.ping")
	{
		BOOST_LOG_SEV(lg_, ls::info) << "Received rpc control ping";
		send_reply_control_ping(request);
	}
	else if(request["method"] == "v0.env.reset")
	{
		BOOST_LOG_SEV(lg_, ls::warning) <<  "Received reset request from agent";
		reset_signal_(request);
	}
	else if(request["method"] == "v0.agent.action")
	{
		BOOST_LOG_SEV(lg_, ls::debug) << "Received action from agent";
		action_signal_(request);
	}
}

void AgentConn::sendJson_(std::string const& method, Json::Value const& body, int episode_id, const long long parent_message_id)
{
	Json::Value payload;
	payload["method"] = method;
	payload["body"] = body;
	payload["headers"] = get_headers_(parent_message_id, episode_id);
	std::string msg_string = json_dumps(payload);
	if(m_connections_.size() < 1)
	{
		BOOST_LOG_SEV(lg_, ls::info) << "No clients connected -- skipping message";
		no_clients_signal_();
		return;
	}
	websocketSend(&websocket_server_, most_recent_websocket_cxn_, msg_string, lg_);
}
