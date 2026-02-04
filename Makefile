.PHONY: help build clean robot

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

# Robot target - runs the built robot program in Nix container
robot: build ## Build and run the robot program
	podman run --rm -v $(PWD):/workspace -w /workspace robot-build ./build/robot