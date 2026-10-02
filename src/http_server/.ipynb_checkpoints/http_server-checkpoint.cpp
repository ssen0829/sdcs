#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <chrono>
#include <sstream>

#include "sdcs/types.h"
#include "sdcs/cluster.h"

// 引入 httplib
#if __has_include(<httplib.h>)
#include <httplib.h>
#elif __has_include("httplib.h")
#include "httplib.h"
#elif __has_include("httplib/httplib.h")
#include "httplib/httplib.h"
#else
#include <httplib.h>
#endif

namespace sdcs {

static std::string UrlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.size());
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            int value = 0;
            std::istringstream hex_stream(str.substr(i + 1, 2));
            if (hex_stream >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += str[i];
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

void StartHttpServer(const std::string& host, int port, std::shared_ptr<ClusterService> cluster) {
    httplib::Server svr;

    // GET 方法
    svr.Get(".*", [cluster](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json; charset=utf-8");
        
        if (req.target == "/ping") {
            auto health = cluster->Health();
            if (health.accepting_requests) {
                res.status = 200;
                res.body = R"({"status":"pong"})";
            } else {
                res.status = 503;
                res.body = R"({"status":"not_ready"})";
            }
            return;
        }

        if (req.path.length() <= 1) {
            res.status = 400;
            res.body = R"({"error":{"code":"INVALID_ARGUMENT","message":"Missing key"}})";
            return;
        }

        std::string raw_key = req.path.substr(1);
        std::string key = UrlDecode(raw_key);

        RequestContext ctx;
        ctx.request_id = "req-1";
        ctx.deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);

        auto result = cluster->Get(key, ctx);
        if (result.status.code == Code::Ok && result.value.has_value()) {
            res.status = 200;
            res.body = "{\"" + key + "\":" + result.value.value().dump() + "}";
        } else if (result.status.code == Code::NotFound) {
            res.status = 404;
            res.body = "";
        } else {
            res.status = 500;
            res.body = R"({"error":{"code":"INTERNAL","message":"Error"}})";
        }
    });

    // POST 方法
    svr.Post("/", [cluster](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json; charset=utf-8");
        try {
            nlohmann::json body_json = nlohmann::json::parse(req.body);

            RequestContext ctx;
            ctx.request_id = "req-1";
            ctx.deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);

            auto result = cluster->Put("key", body_json, ctx);
            if (result.status.code == Code::Ok) {
                res.status = 200;
                res.body = "{}";
            } else {
                res.status = 500;
                res.body = R"({"error":{"code":"INTERNAL","message":"Error"}})";
            }
        } catch (...) {
            res.status = 400;
            res.body = R"({"error":{"code":"INVALID_ARGUMENT","message":"Invalid JSON"}})";
        }
    });

    // DELETE 方法
    svr.Delete(".*", [cluster](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json; charset=utf-8");
        if (req.path.length() <= 1) {
            res.status = 400;
            res.body = R"({"error":{"code":"INVALID_ARGUMENT","message":"Missing key"}})";
            return;
        }

        std::string raw_key = req.path.substr(1);
        std::string key = UrlDecode(raw_key);

        RequestContext ctx;
        ctx.request_id = "req-1";
        ctx.deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);

        auto result = cluster->Delete(key, ctx);
        if (result.status.code == Code::Ok) {
            res.status = 200;
            res.body = std::to_string(result.deleted);
        } else {
            res.status = 500;
            res.body = R"({"error":{"code":"INTERNAL","message":"Error"}})";
        }
    });

    svr.listen(host.c_str(), port);
}

}  // namespace sdcs