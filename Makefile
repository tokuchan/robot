.PHONY: help build clean robot test test-filter docs intellisense format

# Default target
.DEFAULT_GOAL := help

# Help target - shows usage and task table
help: ## Show this help message
	@echo "Usage: make [target]"
	@echo ""
	@echo "Available tasks:"
	@grep -E '^[a-zA-Z_-]+:.*?## ' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "} {printf "  %-15s %s\n", $$1, $$2}'

# Build target - uses Dockerfile to invoke cmake in Nix environment
build: ## Build the robot program using cmake in Nix dev environment
	podman build --security-opt label=disable -t robot-build .
	podman run --rm -v $(PWD):/workspace -w /workspace robot-build nix develop --command bash -c "cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && cmake --build build"

# IntelliSense target - fix compile_commands.json paths for host IDE
intellisense: build ## Fix compile_commands.json paths for IntelliSense
	sed 's|/workspace|$(PWD)|g' build/compile_commands.json > build/compile_commands_host.json
	@podman run --rm -v $(PWD):/workspace -w /workspace robot-build nix develop --command bash -c "echo '#include <...> search starts here:' && g++ -E -x c++ - -v < /dev/null 2>&1 | sed -n '/search starts here:/,/End of search/p'" > build/nix_include_paths.txt
	@echo "Generated build/compile_commands_host.json and build/nix_include_paths.txt for IntelliSense"

shell: ## Open a shell inside the build environment
	podman build --security-opt label=disable -t robot-build .
	podman run --rm -it -v $(PWD):/workspace -w /workspace robot-build nix develop --command bash

# Clean target - removes build products
clean: ## Remove build artifacts and temporary files
	rm -rf build
	rm -f robot
	rm -rf docs
	rm -f doxygen_sqlite3.db

# Test target - runs the test suite
test: build ## Run the test suite
	podman run --rm -v $(PWD):/workspace -w /workspace robot-build nix develop --command ./build/robot_tests

# Test with filter - runs tests matching a pattern (usage: make test-filter FILTER="pattern")
test-filter: build ## Run tests matching a filter (make test-filter FILTER="pattern")
	podman run --rm -v $(PWD):/workspace -w /workspace robot-build nix develop --command ./build/robot_tests "$(FILTER)"

# Robot target - runs the built robot program in Nix container
robot: build ## Build and run the robot program
	podman run --rm -p 8080:8080 -v $(PWD):/workspace -w /workspace robot-build ./build/robot

# Docs target - regenerates documentation
docs: ## Regenerate Doxygen documentation
	podman build --security-opt label=disable -t robot-build .
	podman run --rm -v $(PWD):/workspace -w /workspace robot-build nix develop --command doxygen Doxyfile

# Format target - reformats all source files using clang-format
format: ## Format all C++ source files using clang-format
	podman build --security-opt label=disable -t robot-build .
	podman run --rm -v $(PWD):/workspace -w /workspace robot-build nix develop --command bash -c "find src include tests -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i"
