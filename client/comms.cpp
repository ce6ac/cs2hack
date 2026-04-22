#include "comms.h"
#include <iostream>
#include <ixwebsocket/IXWebSocket.h>
#include <atomic>

bool communications::ws_connect(const std::string& url) {
    m_ws.setUrl(url);

    m_ws.enableAutomaticReconnection();

    m_ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            m_connected = true;
            std::cout << "webapp: websocket connected" << std::endl;
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            m_connected = false;
            std::cout << "webapp: websocket closed" << std::endl;
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            m_connected = false;
            std::cerr << "webapp: websocket error - " << msg->errorInfo.reason << std::endl;
        }
    });

    m_ws.start();

    // wait up to 3 s for the connection to open
    for (int i = 0; i < 30; ++i) {
        if (m_connected) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cerr << "webapp: websocket connection timed out" << std::endl;
    return false;
}

bool communications::ws_send(const nlohmann::json& data) {
    if (!m_connected) return false;
    auto result = m_ws.send(data.dump());
    return result.success;
}

void communications::ws_disconnect() {
    m_ws.stop();
    m_connected = false;
}

bool communications::ws_connected() const {
    return m_connected;
}

size_t communications::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t totalsize = size * nmemb;
	std::string* resp = static_cast<std::string*>(userp);
	if (resp) {
		resp->append(static_cast<char*>(contents), totalsize);
		return totalsize;
	}
	return 0;
}

std::string communications::get_data(const std::string& url) {
	try {
		CURL* curl = curl_easy_init();
		std::string resp;

		if (curl) {
			CURLcode res;

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, communications::write_callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);

			res = curl_easy_perform(curl);

			if (res != CURLE_OK) {
				std::cerr << "(curl error) - " << curl_easy_strerror(res);
			} else if (resp.empty()) {
				std::cerr << "no data received from the server" << std::endl;
			}
			curl_easy_cleanup(curl);
		} else {
			std::cerr << "failed to initialize curl locally" << std::endl;
		}

		return resp;
	} catch (const std::exception& e) {
		std::cerr << "exception - " << e.what() << std::endl;
		return "";
	}
}

std::string communications::post_data(const nlohmann::json& data, const std::string& url) {
	try {
		CURL* curl = curl_easy_init();
		std::string response;

		if (curl) {
			CURLcode res;
			std::string jsonString = data.dump();

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());

			struct curl_slist* headers = nullptr;
			headers = curl_slist_append(headers, "Content-Type: application/json");
			if (headers == nullptr) {
				std::cerr << "failed to alloc memory for headers in curl" << std::endl;
				curl_easy_cleanup(curl);
				return "";
			}
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, communications::write_callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

			res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				std::cerr << "(curl error) - " << curl_easy_strerror(res);
			}

			curl_slist_free_all(headers);
			curl_easy_cleanup(curl);
		} else {
			std::cerr << "failed to initialize curl locally" << std::endl;
			return "";
		}
		return response;
	} catch (const std::exception& e) {
		std::cerr << "exception - " << e.what() << std::endl;
		return "";
	}
}
