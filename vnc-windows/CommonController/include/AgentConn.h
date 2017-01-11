#ifndef AGENT_CONN_H_
#define AGENT_CONN_H_

#include <json/json.h>
#include <memory>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <mutex>
#include <set>
#include <thread>
#include <boost/signals2.hpp>
#include <Common.h>

typedef websocketpp::server<websocketpp::config::asio> server;
typedef boost::signals2::signal<void (const Json::Value agent_request)>  ResetSignal;
typedef boost::signals2::signal<void (const Json::Value agent_request)>  ActionSignal;
typedef boost::signals2::signal<void ()> NoClientsSignal;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

class AgentConn
{
public:
	AgentConn(int port, boost::log::sources::severity_logger_mt<ls::severity_level> lg);
	~AgentConn();
	void send_env_describe(std::string const& env_id, std::string const& env_state, int episode_id, int fps, std::string const& metadata="");
	void send_reply_control_ping(Json::Value const& request);
	void send_reset_reply(Json::Value const& request, int episode_id);
	void sendReward(const double reward, int episode_id, const bool done, Json::Value info=Json::Value(Json::ValueType::objectValue));
	bool client_is_connected();
	boost::signals2::connection on_reset(const ResetSignal::slot_type &subscriber);
	boost::signals2::connection on_action(const ActionSignal::slot_type& subscriber);
	boost::signals2::connection on_no_clients(const NoClientsSignal::slot_type& subscriber);
private:
	int port_;
	server websocket_server_;
	std::unique_ptr<std::thread> server_thread_;
	websocketpp::connection_hdl most_recent_websocket_cxn_;
	typedef std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> con_list;
	con_list m_connections_;
	std::mutex m_connection_lock_;
	boost::log::sources::severity_logger_mt<ls::severity_level> lg_;

	void on_websocket_open_(websocketpp::connection_hdl websocket_cxn);
	void on_websocket_close_(websocketpp::connection_hdl websocket_cxn);
	void on_websocket_msg_(websocketpp::connection_hdl websocket_cxn, message_ptr msg);
	void run_server_thread();
	Json::Value get_headers_(const long long message_id, int episode_id);
	void sendJson_(std::string const& method, Json::Value const& body, int episode_id, const long long parent_message_id=-1);
	void populate_sent_at_in_headers_(Json::Value& headers);
	ResetSignal reset_signal_;
	ActionSignal action_signal_;
	NoClientsSignal no_clients_signal_;
};

Json::Value json_loads(std::string const& json_string);
std::string json_dumps(Json::Value const& root);

#endif // !AGENT_CONN_H_
