#include <string>
#include <curl/curl.h>
#include <ixwebsocket/IXWebSocket.h>
#include "include/json.hpp"

class communications {
public:
	std::string get_data(const std::string& url);
	std::string post_data(const nlohmann::json& data, const std::string& url);
	bool ws_connect(const std::string& url);
    bool ws_send(const nlohmann::json& data);
    void ws_disconnect();
    bool ws_connected() const;

private:
	static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
	ix::WebSocket m_ws;
    std::atomic<bool> m_connected{ false };
};