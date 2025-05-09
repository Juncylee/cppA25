#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

using json = nlohmann::json;

// Helper: remove markdown characters (#, *)
static std::string cleanMarkdown(const std::string &input) {
    std::string output;
    for (char c : input) {
        if (c != '#' && c != '*') {
            output += c;
        }
    }
    return output;
}

// Buffer for SSE parsing
static std::string streamBuffer;

// Callback for streaming output with SSE parsing and CRLF normalization
static size_t StreamCallback(void* contents, size_t size, size_t nmemb, void* /*userp*/) {
    size_t total = size * nmemb;
    // Append incoming data to buffer
    streamBuffer.append((char*)contents, total);
    // Normalize CRLF to LF
    size_t pos_crlf;
    while ((pos_crlf = streamBuffer.find("\r\n")) != std::string::npos) {
        streamBuffer.replace(pos_crlf, 2, "\n");
    }
    // Process complete SSE events separated by double newline
    size_t pos;
    while ((pos = streamBuffer.find("\n\n")) != std::string::npos) {
        std::string event = streamBuffer.substr(0, pos);
        streamBuffer.erase(0, pos + 2);
        std::istringstream ss(event);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.rfind("data: ", 0) == 0) {
                std::string payload = line.substr(6);
                if (payload == "[DONE]") return total;
                try {
                    auto chunk = json::parse(payload);
                    auto delta = chunk["choices"][0]["delta"];
                    // Only print non-empty content or reasoning_content
                    if (delta.contains("content") && delta["content"].is_string() && !delta["content"].get<std::string>().empty()) {
                        std::cout << delta["content"].get<std::string>();
                    } else if (delta.contains("reasoning_content") && delta["reasoning_content"].is_string() && !delta["reasoning_content"].get<std::string>().empty()) {
                        std::cout << delta["reasoning_content"].get<std::string>();
                    }
                    std::cout.flush();
                } catch (const std::exception&) {
                    // JSON parse error, skip
                }
            }
        }
    }
    return total;
}

std::string loadApiKey() {
    std::ifstream ifs("apikey.txt");
    std::string key;
    std::getline(ifs, key);
    if (key.empty()) {
        std::cerr << "[错误] 无法读取 API Key，请检查 apikey.txt 是否存在并且不为空。" << std::endl;
    }
    return key;
}

void callDeepSeekStream(const json &analysis) {
    std::string key = loadApiKey();
    if (key.empty()) return;

    json req = {
        {"model", "deepseek-reasoner"},
        {"stream", true},
        {"messages", json::array({
            {{"role","system"}, {"content","你是理财助手，请基于以下JSON数据生成总结建议。"}},
            {{"role","user"}, {"content", analysis.dump()}}
        })}
    };

    std::string reqBody = req.dump();
    CURL* curl = curl_easy_init();
    struct curl_slist* hdr = nullptr;
    hdr = curl_slist_append(hdr, ("Authorization: Bearer " + key).c_str());
    hdr = curl_slist_append(hdr, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.deepseek.com/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdr);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reqBody.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, reqBody.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StreamCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");  

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "[错误] CURL 请求失败: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(hdr);
    curl_easy_cleanup(curl);
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U8TEXT);
#endif

    std::ifstream summaryFile("data_summary.txt");
    if (summaryFile) {
        std::cout << "=====================================" << std::endl;
        std::cout << "             交易汇总报告            " << std::endl;
        std::cout << "=====================================" << std::endl;

        std::string line;
        while (std::getline(summaryFile, line)) {
            if (!line.empty()) {
                std::cout << "  " << line << std::endl;
            }
        }
        std::cout << "=====================================" << std::endl;
    } else {
        std::cerr << "[错误] 无法读取 data_summary.txt，请先运行 data_extractor.py" << std::endl;
    }

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        std::cerr << "CURL 全局初始化失败" << std::endl;
        return 1;
    }

    std::ifstream ifs("data.json");
    if (!ifs) {
        std::cerr << "[错误] 请先运行 data_extractor.py 生成 data.json" << std::endl;
        return 1;
    }

    json analysis;
    ifs >> analysis;

    std::cout << "               AI汇总报告            " << std::endl;
    std::cout << "=====================================" << std::endl;
    callDeepSeekStream(analysis);
    std::cout << std::endl;
    std::cout << "=====================================" << std::endl;

    curl_global_cleanup();
    return 0;
}
