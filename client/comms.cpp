#include "comms.h"
#include <iostream>

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
	CURL* curl = curl_easy_init();
	std::string resp;

	if (curl) {
		CURLcode res;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, this->write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			std::cerr << "webapp: (curl error) - " << curl_easy_strerror(res) << std::endl;
		} else if (resp.empty()) {
			std::cerr << "webapp: no data received from the server" << std::endl;
		}
		curl_easy_cleanup(curl);
	} else {
		std::cerr << "webapp: failed to initialize curl locally" << std::endl;
	}

	return resp;
}

std::string communications::post_data(const nlohmann::json& data, const std::string& url) {
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
			std::cerr << "webapp: failed to alloc memory for headers in curl" << std::endl;
			curl_easy_cleanup(curl);
			return "";
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, this->write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			std::cerr << "webapp: (curl error) - " << curl_easy_strerror(res) << std::endl;
		}

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	} else {
		std::cerr << "webapp: failed to initialize curl locally" << std::endl;
	}

	return response;
}
