#include <string>
#include <curl/curl.h>
#include "include/json.hpp"

class communications {
public:
	std::string get_data(const std::string& url);
	std::string post_data(const nlohmann::json& data, const std::string& url);

private:
	static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};