#include "shell/CommandParser.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace advshell {

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

std::string expandVariables(std::string_view input, int lastExitCode) {
    std::string out;
    bool inSingleQuotes = false;

    for (std::size_t i = 0; i < input.size(); ++i) {
        const char c = input[i];
        if (c == '\'' && (i == 0 || input[i - 1] != '\\')) {
            inSingleQuotes = !inSingleQuotes;
            out += c;
            continue;
        }
        if (inSingleQuotes || c != '$' || i + 1 >= input.size()) {
            out += c;
            continue;
        }

        if (input[i + 1] == '?') {
            out += std::to_string(lastExitCode);
            ++i;
            continue;
        }

        std::size_t start = i + 1;
        bool braced = false;
        if (input[start] == '{') {
            braced = true;
            ++start;
        }

        std::size_t end = start;
        while (end < input.size() &&
               (std::isalnum(static_cast<unsigned char>(input[end])) || input[end] == '_')) {
            ++end;
        }

        if (end > start) {
            const std::string varName(input.substr(start, end - start));
            const char* val = std::getenv(varName.c_str());
            if (val) out += val;
            if (braced && end < input.size() && input[end] == '}') {
                ++end;
            }
            i = end - 1;
        } else {
            out += c;
        }
    }
    return out;
}

std::vector<std::string> tokenize(std::string_view input) {
    std::vector<std::string> tokens;
    std::string current;
    bool inSingle = false;
    bool inDouble = false;

    auto flushCurrent = [&]() {
        if (!current.empty()) {
            tokens.push_back(std::move(current));
            current.clear();
        }
    };

    for (std::size_t i = 0; i < input.size(); ++i) {
        const char c = input[i];

        if (c == '\\' && !inSingle && i + 1 < input.size()) {
            current += input[++i];
            continue;
        }
        if (c == '\'' && !inDouble) {
            inSingle = !inSingle;
            continue;
        }
        if (c == '"' && !inSingle) {
            inDouble = !inDouble;
            continue;
        }

        if (!inSingle && !inDouble) {
            if (c == '#') {
                break; // comment to end of line
            }
            if (std::isspace(static_cast<unsigned char>(c))) {
                flushCurrent();
                continue;
            }
            if (c == '|' || c == '<' || c == '&') {
                flushCurrent();
                tokens.emplace_back(1, c);
                continue;
            }
            if (c == '>') {
                flushCurrent();
                if (i + 1 < input.size() && input[i + 1] == '>') {
                    tokens.emplace_back(">>");
                    ++i;
                } else {
                    tokens.emplace_back(">");
                }
                continue;
            }
        }

        current += c;
    }

    flushCurrent();
    return tokens;
}

ParsedLine parseCommandLine(std::string_view input,
                            const std::map<std::string, std::string>& aliases,
                            int lastExitCode) {
    ParsedLine result;
    const auto trimmedInput = trim(input);
    if (trimmedInput.empty()) return result;

    const std::string expanded = expandVariables(trimmedInput, lastExitCode);
    std::vector<std::string> tokens = tokenize(expanded);
    if (tokens.empty()) return result;

    // Expand leading alias on the first command token if present
    auto aliasIt = aliases.find(tokens.front());
    if (aliasIt != aliases.end()) {
        auto aliasTokens = tokenize(aliasIt->second);
        aliasTokens.insert(aliasTokens.end(), tokens.begin() + 1, tokens.end());
        tokens = std::move(aliasTokens);
    }

    if (!tokens.empty() && tokens.back() == "&") {
        result.background = true;
        tokens.pop_back();
    }

    CommandStage stage;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const std::string& tok = tokens[i];
        if (tok == "|") {
            if (stage.args.empty()) {
                result.errorMessage = "Syntax error: empty command in pipeline.";
                result.pipeline.clear();
                return result;
            }
            result.pipeline.push_back(std::move(stage));
            stage = CommandStage{};
        } else if (tok == "<") {
            if (i + 1 >= tokens.size()) {
                result.errorMessage = "Syntax error: expected filename after '<'.";
                result.pipeline.clear();
                return result;
            }
            stage.inputFile = tokens[++i];
        } else if (tok == ">" || tok == ">>") {
            if (i + 1 >= tokens.size()) {
                result.errorMessage = "Syntax error: expected filename after '" + tok + "'.";
                result.pipeline.clear();
                return result;
            }
            stage.outputFile = tokens[++i];
            stage.appendOutput = (tok == ">>");
        } else {
            stage.args.push_back(tok);
        }
    }

    if (stage.args.empty() && !result.pipeline.empty()) {
        result.errorMessage = "Syntax error: trailing pipe '|'.";
        result.pipeline.clear();
        return result;
    }

    if (!stage.args.empty()) {
        result.pipeline.push_back(std::move(stage));
    }
    return result;
}

} // namespace advshell

