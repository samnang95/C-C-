#include "http/HttpMessage.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace httpserver {

std::string_view trim(std::string_view text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::string lowercase(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

std::string uppercase(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return result;
}

std::string statusReasonPhrase(int statusCode) {
    switch (statusCode) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default:  return "OK";
    }
}

std::string guessMimeType(std::string_view path) {
    const auto dotPos = path.find_last_of('.');
    if (dotPos == std::string_view::npos) return "text/plain; charset=utf-8";
    const auto ext = lowercase(path.substr(dotPos));
    if (ext == ".html" || ext == ".htm") return "text/html; charset=utf-8";
    if (ext == ".css") return "text/css; charset=utf-8";
    if (ext == ".js") return "application/javascript; charset=utf-8";
    if (ext == ".json") return "application/json; charset=utf-8";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    return "text/plain; charset=utf-8";
}

bool isSafeRelativePath(std::string_view path) {
    if (path.empty() || path.front() != '/') return false;
    if (path.find("..") != std::string_view::npos) return false;
    if (path.find('\\') != std::string_view::npos) return false;
    return true;
}

std::string HttpResponse::serialize() const {
    std::ostringstream out;
    const std::string phrase = statusText.empty() ? statusReasonPhrase(statusCode) : statusText;
    out << "HTTP/1.1 " << statusCode << ' ' << phrase << "\r\n";
    out << "Content-Type: " << contentType << "\r\n";
    out << "Content-Length: " << body.size() << "\r\n";
    out << "Connection: close\r\n";
    for (const auto& [k, v] : headers) {
        out << k << ": " << v << "\r\n";
    }
    out << "\r\n" << body;
    return out.str();
}

bool parseHttpRequest(std::string_view raw, HttpRequest& outRequest) {
    outRequest = HttpRequest{};

    const auto headerEnd = raw.find("\r\n\r\n");
    const std::size_t sepLen = (headerEnd != std::string_view::npos) ? 4 : 2;
    const auto altEnd = (headerEnd != std::string_view::npos) ? headerEnd : raw.find("\n\n");

    std::string_view headerBlock = (altEnd != std::string_view::npos) ? raw.substr(0, altEnd) : raw;
    if (altEnd != std::string_view::npos && altEnd + sepLen <= raw.size()) {
        outRequest.body = std::string(raw.substr(altEnd + sepLen));
    }

    std::istringstream stream{std::string(headerBlock)};
    std::string requestLine;
    if (!std::getline(stream, requestLine)) return false;
    if (!requestLine.empty() && requestLine.back() == '\r') requestLine.pop_back();

    std::istringstream rlStream(requestLine);
    std::string target;
    if (!(rlStream >> outRequest.method >> target)) return false;
    if (!(rlStream >> outRequest.version)) {
        outRequest.version = "HTTP/1.1";
    }

    outRequest.method = uppercase(trim(outRequest.method));
    if (outRequest.method.empty() || target.empty() || target.front() != '/') {
        return false;
    }

    const auto qPos = target.find('?');
    if (qPos != std::string::npos) {
        outRequest.path = target.substr(0, qPos);
        outRequest.query = target.substr(qPos + 1);
    } else {
        outRequest.path = target;
    }

    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;
        const auto colon = line.find(':');
        if (colon == std::string::npos) continue;
        const auto key = lowercase(trim(std::string_view(line).substr(0, colon)));
        const auto val = std::string(trim(std::string_view(line).substr(colon + 1)));
        if (!key.empty()) {
            outRequest.headers[key] = val;
        }
    }

    return true;
}

} // namespace httpserver

