.PHONY: help build clean robot test test-filter

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
	podman run --rm -v $(PWD):/workspace -w /workspace robot-build nix develop --command bash -c "rm -rf build && cmake -B build && cmake --build build"

# Clean target - removes build products
clean: ## Remove build artifacts and temporary files
	rm -rf build
	rm -f robot

# Test target - runs the test suite
test: build ## Run the test suite
	podman run --rm -v $(PWD):/workspace -w /workspace robot-build nix develop --command ./build/robot_tests

# Test with filter - runs tests matching a pattern (usage: make test-filter FILTER="pattern")
test-filter: build ## Run tests matching a filter (make test-filter FILTER="pattern")
	podman run --rm -v $(PWD):/workspace -w /workspace robot-build nix develop --command ./build/robot_tests "$(FILTER)"

# Robot target - runs the built robot program in Nix container
robot: build ## Build and run the robot program
	podman run --rm -v $(PWD):/workspace -w /workspace robot-build ./build/robot