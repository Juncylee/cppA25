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

using namespace std;
using json = nlohmann::json;

// 用于 SSE 解析的缓冲区
static string streamBuffer;
// 用于处理 SSE 输出并规范化 CRLF 的回调函数
static size_t StreamCallback(void* contents, size_t size, size_t nmemb, void* /*userp*/)
{
    size_t total = size * nmemb;
    // 将接收到的数据追加到缓冲区
    streamBuffer.append((char*)contents, total);
    // 将 CRLF 转换为 LF
    size_t pos_crlf;
    while ((pos_crlf = streamBuffer.find("\r\n")) != string::npos)
    {
        streamBuffer.replace(pos_crlf, 2, "\n");
    }

    static bool separator_printed = false;

    // 处理以双换行分隔的完整 SSE 事件
    size_t pos;
    while ((pos = streamBuffer.find("\n\n")) != string::npos)
    {
        string event = streamBuffer.substr(0, pos);
        streamBuffer.erase(0, pos + 2);
        istringstream shuju(event);
        string line;
        while (getline(shuju, line))
        {
            if (line.rfind("data: ", 0) == 0)
            {
                string payload = line.substr(6);
                if (payload == "[DONE]") return total;
                try
                {
                    auto chunk = json::parse(payload);
                    auto delta = chunk["choices"][0]["delta"];

                    if (!separator_printed
                        && delta.contains("content")
                        && delta["content"].is_string()
                        && !delta["content"].get<string>().empty())
                    {
                        cout << "\n\n\n---------------------------\n\n\n";
                        separator_printed = true;
                    }
                    // 仅输出非空的 content 或 reasoning_content
                    if (delta.contains("content")
                        && delta["content"].is_string() 
                        && !delta["content"].get<string>().empty())
                    {
                        cout << delta["content"].get<string>();
                    } 
                    else if (delta.contains("reasoning_content") 
                             && delta["reasoning_content"].is_string() 
                             && !delta["reasoning_content"].get<string>().empty())
                    {
                        cout << delta["reasoning_content"].get<string>();
                    }
                    cout.flush();
                }
                catch (const exception&) 
                {
                    cout << "[错误] 无法解析 JSON 数据" << endl;
                }
            }
        }
    }
    return total;
}

// 加载 API Key
string loadApiKey()
{
    ifstream ifs("apikey.txt");
    string key;
    getline(ifs, key);
    if (key.empty())
    {
        cout << "[错误] 无法读取 API Key，请检查 apikey.txt 是否存在并且不为空。" << endl;
    }
    return key;
}

// 调用 DeepSeek 的流式接口
void callDeepSeekStream(const json &analysis)
{
    string key = loadApiKey();
    if (key.empty()) return;

    json req = {
        {"model", "deepseek-reasoner"},
        {"stream", true},
        {"messages", json::array({
            {{"role","system"}, {"content","你是理财助手，请基于以下数据给出合理并详细的总结建议，回答时禁用markdown语法。"}},
            {{"role","user"}, {"content", analysis.dump()}}
        })}
    };

    string reqBody = req.dump();
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

    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");  // 根证书验证

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        cout << "[错误] CURL 请求失败: " << curl_easy_strerror(res) << endl;
    }

    curl_slist_free_all(hdr);
    curl_easy_cleanup(curl);
}

// 清除 Markdown 语法字符
static string cleanMarkdown(const string &input)
{
    string output;
    for (char c : input) {
        if (c != '#' && c != '*') {
            output += c;
        }
    }
    return output;
}

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U8TEXT);
    #endif

    try {
        // 读取 dataex.txt 全文
        ifstream ifs("dataex.txt");
        if (!ifs.is_open()) {
            cerr << "[错误] 无法打开 dataex.txt，请确保文件存在于可执行文件同级目录。" << endl;
            return 1;
        }
        ostringstream buffer;
        buffer << ifs.rdbuf();
        string rawData = buffer.str();

        cout << "===========================" << endl;
        cout << "\n" ;
        cout << "       交易汇总报告        " << endl;
        cout << "\n" << rawData << "\n";
        cout << "        AI汇总报告         " << endl;
        cout << "  （DeepSeek-R1深度思考）  " << endl;
        cout << "\n" ;
        cout << "===========================" << endl;
        cout << "\n" ;

        // 构造 JSON 对象，字段名可根据 API 要求调整
        json analysisPayload = {
            {"data", rawData}
        };

        // 调用 DeepSeek 流式接口
        callDeepSeekStream(analysisPayload);

    } catch (const exception& ex) {
        cerr << "[异常] " << ex.what() << endl;
        return 1;
    }

    return 0;
}
