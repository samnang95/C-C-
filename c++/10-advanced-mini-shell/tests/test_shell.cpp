#include "shell/CommandParser.hpp"
#include "shell/ShellEngine.hpp"
#include "ui/ConsoleUI.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

int main() {
    using namespace advshell;

    // 1. Variable expansion & tokenizer
    ::setenv("MINI_SH_TEST", "hello_world", 1);
    assert(expandVariables("val=$MINI_SH_TEST code=$?", 42) == "val=hello_world code=42");
    assert(expandVariables("'$MINI_SH_TEST'", 0) == "'$MINI_SH_TEST'");

    const auto toks = tokenize(R"(echo "hello world" | grep hello >> out.txt &)");
    assert(toks.size() == 8);
    assert(toks[1] == "hello world");
    assert(toks[2] == "|");
    assert(toks[5] == ">>");
    assert(toks[7] == "&");

    // 2. Pipeline, redirection, and alias parsing
    std::map<std::string, std::string> aliases{{"ll", "ls -la"}};
    const auto parsed = parseCommandLine("ll < in.txt | wc -l > count.txt &", aliases, 0);
    assert(parsed.errorMessage.empty());
    assert(parsed.background);
    assert(parsed.pipeline.size() == 2);
    assert(parsed.pipeline[0].args.size() == 2 && parsed.pipeline[0].args[0] == "ls");
    assert(parsed.pipeline[0].inputFile == "in.txt");
    assert(parsed.pipeline[1].args[0] == "wc");
    assert(parsed.pipeline[1].outputFile == "count.txt");
    assert(!parsed.pipeline[1].appendOutput);

    // 3. ShellEngine execution, pipelines, redirection, and history persistence
    const std::string testHist = "/tmp/test_adv_shell_history.db";
    const std::string testOut = "/tmp/test_adv_shell_out.txt";
    std::remove(testHist.c_str());
    std::remove(testOut.c_str());

    {
        ShellEngine engine(testHist);
        assert(engine.executeLine("export GREETING=Testing123"));
        assert(engine.executeLine("echo $GREETING > " + testOut));
        assert(engine.lastExitCode() == 0);

        std::ifstream in(testOut);
        std::string line;
        std::getline(in, line);
        assert(line == "Testing123");

        assert(engine.saveHistory());
    }

    {
        ShellEngine loaded(testHist);
        assert(loaded.loadHistory());
        assert(loaded.history().size() == 2);
        assert(!loaded.executeLine("exit"));
    }

    std::remove(testHist.c_str());
    std::remove(testOut.c_str());
    std::cout << "All advanced mini shell unit tests passed!\n";
    return 0;
}

