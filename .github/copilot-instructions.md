# Copilot instructions for the `robot` repository

Short, actionable notes to help AI agents be productive in this C++/CMake project.

1. Project summary
- Language: C++ (C++23). Small simulation backend producing `robot` executable.
- Build system: CMake driven via the top-level `Makefile` which builds inside a Nix container image using `podman`.

2. Quick commands (use `make` from repository root)
- `make build` — builds the project (invokes containerized CMake/Nix flow).
- `make test` — builds and runs the full Catch2 test binary (`build/robot_tests`).
- `make test-filter FILTER="pattern"` — run tests filtered by `pattern`.
- `make robot` — build and run the `robot` executable inside the same container environment.

3. Key files and what they contain
- [CMakeLists.txt](CMakeLists.txt): project config, finds `Boost` and `Catch2`, creates `robot` and `robot_tests` targets.
- [src/robot.cpp](src/robot.cpp): program entrypoint; sets up SIGINT handler and calls `mainloop`.
- [src/mainloop.hpp](src/mainloop.hpp): contains `mainloop(std::stop_source&)` implementation using `std::jthread` and `stop_token`.
- [include/robot.hpp](include/robot.hpp): small utility declarations used by tests.
- [tests/](tests): Catch2-based unit tests (compiled into `robot_tests`).
- [Makefile](Makefile): primary developer-facing task list; builds run inside a `robot-build` container image and require `podman` and `nix` on the host.

4. Architecture & runtime notes (concise)
- Single-process backend with a long-running event loop implemented in `mainloop`. The loop uses `std::jthread` and cooperative cancellation via `std::stop_token`/`std::stop_source`.
- Signal management: `src/robot.cpp` installs a SIGINT handler that requests stop on the global `stop_source`.
- Tests are simple Catch2 tests compiled as a separate test binary (`robot_tests`) and executed directly.

5. Project-specific conventions and patterns
- Containerized development: builds/tests are executed inside a Nix dev environment via `podman` images created by `Makefile`. Do not assume native system packages are available — use `make` targets.
- Header layout: public headers live in `include/` and are added to targets via `target_include_directories(... PUBLIC include)`.
- Tests live in `tests/` and use Catch2. The test binary is run directly (no additional test harness).
- Small, synchronous main loop: `mainloop` currently busy-waits; add work to the lambda body and respect `stop_token.stop_requested()` for clean shutdown.

6. Common tasks examples
- Add a new source file `src/foo.cpp`, declare API in `include/foo.hpp`, add it to the `robot` target by modifying `CMakeLists.txt` (or include via target_sources), then run `make build`.
- To run a single test locally: `make test-filter FILTER="approval"` (uses Catch2 filter semantics).

7. Environment prerequisites
- `podman` (or Docker compatible), `nix`, `make` are expected for the supported `Makefile` workflows.

8. When you edit code
- Prefer minimal, focused changes. Keep public API changes (headers) explicit and update `CMakeLists.txt` if adding new targets or source files.
- Avoid changing container/build scripts unless necessary; update `Makefile` only when adding new developer tasks.

9. Where to look next for context
- [README.md](README.md) — high-level goals and design rationale (contains placeholders).
- `build/` — generated build artifacts and CMake cache if you need to inspect output paths.

If any part is unclear or you want extra sections (debugging tips, test patterns, CI hooks), tell me which area to expand.
