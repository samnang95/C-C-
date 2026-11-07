# Mini HTTP Server (C++)

A modular, dependency-free C++17 HTTP/1.1 server and interactive route console. Supports POSIX TCP socket listening (`serve`), static file serving from `data/www/` with MIME-type detection and path-traversal protection, configurable REST routes persisted in `data/routes.db`, interactive HTTP request testing (`request`), and access logging (`logs`).

## Project Structure

```text
08-http-server/
├── build/
├── data/
├── src/
│   ├── main.cpp
│   ├── http/
│   │   ├── HttpMessage.hpp
│   │   ├── HttpMessage.cpp
│   │   ├── HttpServer.hpp
│   │   └── HttpServer.cpp
│   └── ui/
│       ├── ConsoleUI.hpp
│       └── ConsoleUI.cpp
├── tests/
│   └── test_http.cpp
├── .gitignore
├── compile_flags.txt
├── Makefile
└── README.md
```

## Build, Run & Test

```sh
# Build executable into build/http_server
make

# Run interactive server console
make run

# Run unit tests
make test

# Clean build artifacts
make clean
```

## Commands

| Command | Description |
| --- | --- |
| `routes` / `list` | List all configured HTTP endpoints. |
| `add` | Register a new custom HTTP endpoint (`method`, `path`, `status`, `content-type`, `body`). |
| `remove [id]` | Delete a custom HTTP endpoint. |
| `request <METHOD> <path> [body]` | Simulate an HTTP request directly (e.g. `request GET /api/health` or `get /`). |
| `serve [port] [count]` | Start a live TCP server on `127.0.0.1:<port>` for `<count>` requests (default `8080 1`). |
| `logs` | View HTTP access log history (`status`, `method`, `path`, `bytes`). |
| `clear` / `reset` | Clear the terminal screen. |
| `help` | Display command help. |
| `quit` | Save routes and exit. |

