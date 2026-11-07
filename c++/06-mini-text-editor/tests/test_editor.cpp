#include "editor/TextBuffer.hpp"
#include "ui/ConsoleUI.hpp"

#include <cassert>
#include <cstdio>
#include <iostream>

int main() {
    using namespace textedit;

    // 1. Line number & string helpers
    assert(trim("  Hello World \n") == "Hello World");
    assert(lowercase("C++17 Editor") == "c++17 editor");

    std::size_t lineNum = 0;
    assert(parseLineNumber("4", lineNum) && lineNum == 4);
    assert(!parseLineNumber("0", lineNum));
    assert(!parseLineNumber("-2", lineNum));

    // 2. Buffer operations, undo/redo, search/replace, stats, and file persistence
    const std::string testFile = "/tmp/test_mini_text_editor.txt";
    std::remove(testFile.c_str());

    {
        TextBuffer buf(testFile);
        buf.appendLine("First line of text");
        buf.appendLine("Third line of text");
        assert(buf.insertLine(2, "Second line of text"));
        assert(buf.lines().size() == 3);
        assert(buf.lines()[1] == "Second line of text");

        assert(buf.editLine(2, "Updated second line"));
        assert(buf.lines()[1] == "Updated second line");

        // Undo / redo edit
        assert(buf.undo());
        assert(buf.lines()[1] == "Second line of text");
        assert(buf.redo());
        assert(buf.lines()[1] == "Updated second line");

        // Find & replace
        const auto hits = buf.find("line");
        assert(hits.size() == 3);

        const auto replaced = buf.replaceAll("line", "row");
        assert(replaced == 3);
        assert(buf.lines()[0] == "First row of text");

        // Stats
        const auto st = buf.stats();
        assert(st.lineCount == 3);
        assert(st.wordCount == 11);
        assert(st.modified);

        assert(buf.save());
        assert(!buf.isModified());
    }

    {
        TextBuffer loaded(testFile);
        assert(loaded.load());
        assert(loaded.lines().size() == 3);
        assert(loaded.lines()[0] == "First row of text");
        assert(loaded.lines()[1] == "Updated second row");

        assert(loaded.deleteLine(2));
        assert(loaded.lines().size() == 2);
        assert(loaded.undo());
        assert(loaded.lines().size() == 3);
    }

    std::remove(testFile.c_str());
    std::cout << "All mini text editor unit tests passed!\n";
    return 0;
}

