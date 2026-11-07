# Download Manager (C++)

A modular, dependency-free C++17 command-line download queue manager supporting URL filename extraction, live progress bars, chunked transfer stepping, pause/resume/cancel/retry state transitions, and persistent queue state in `data/downloads.db`.

## Project Structure

```text
07-download-manager/
├── build/
├── data/
├── src/
│   ├── main.cpp
│   ├── download/
│   │   ├── DownloadJob.hpp
│   │   ├── DownloadJob.cpp
│   │   ├── DownloadManager.hpp
│   │   └── DownloadManager.cpp
│   └── ui/
│       ├── ConsoleUI.hpp
│       └── ConsoleUI.cpp
├── tests/
│   └── test_download.cpp
├── .gitignore
├── compile_flags.txt
├── Makefile
└── README.md
```

## Build, Run & Test

```sh
# Build executable into build/download_manager
make

# Run interactively
make run

# Run unit tests
make test

# Clean build artifacts
make clean
```

## Commands

| Command | Description |
| --- | --- |
| `add [url]` | Queue a new download job (auto-detects filename from URL). |
| `list [status]` | List all downloads or filter by status (`queued`, `downloading`, `paused`, `completed`, `cancelled`). |
| `view [id]` | Show full details and progress bar for a download. |
| `start [id\|all]` | Run a download (or `all` queued jobs) to completion with a live progress bar. |
| `step [id]` | Advance a download by 25% chunk (useful for testing `pause` / `resume`). |
| `pause [id]` | Pause an active download. |
| `resume [id]` | Resume a paused download. |
| `cancel [id]` | Cancel a download job. |
| `retry [id]` | Reset progress and re-queue a download job. |
| `remove [id]` | Delete a download job from the queue. |
| `search <text>` | Search jobs by URL, filename, or status. |
| `stats` | Display overall download queue statistics and total transferred bytes. |
| `clear` / `reset` | Clear the terminal screen. |
| `help` | Display command help. |
| `quit` | Save queue state and exit. |

