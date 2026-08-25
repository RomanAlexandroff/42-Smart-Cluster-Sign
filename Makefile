##################################################################
# Project Tooling Entry Point for 42 Smart Cluster Sign
#
# This root Makefile exposes the user-facing commands. Larger tools
# live under tools/ and are delegated to their own Makefiles so the
# repository root stays readable as the tooling grows.
##################################################################

SHELL := /bin/bash

SRC_DIR ?= src
FRM_DIR ?= tools/firmware_release_manager

# Neutralize secondary goals for: make format all | make format <file>
ifeq ($(firstword $(MAKECMDGOALS)),format)
  FORMAT_ARG := $(word 2,$(MAKECMDGOALS))
  ifneq ($(FORMAT_ARG),)
    $(eval $(FORMAT_ARG):;@:)
  endif
endif

.PHONY: help release publish-manifest format docs

help:
	@printf '%s\n' \
		'Available targets:' \
		'  make release           Run the Firmware Release Manager.' \
		'  make publish-manifest  Update ota/manifest.json from release-info.json.' \
		'  make format all        Format all .ino/.c/.cpp/.h files under src/.' \
		'  make format <file>     Format one file under src/ (basename or src/<file>).' \
		'  make docs              Build the technical documentation PDF.'

release:
	@$(MAKE) -C "$(FRM_DIR)" ROOT_DIR="$(CURDIR)" release

publish-manifest:
	@$(MAKE) -C "$(FRM_DIR)" ROOT_DIR="$(CURDIR)" publish-manifest

format:
	@set -euo pipefail; \
	command -v clang-format >/dev/null || { echo 'ERROR: clang-format is required.'; exit 1; }; \
	arg="$(FORMAT_ARG)"; \
	if [ -z "$$arg" ]; then \
		echo 'ERROR: missing format target.'; \
		echo 'Usage: make format all'; \
		echo '       make format <file>'; \
		exit 1; \
	fi; \
	files=(); \
	if [ "$$arg" = "all" ]; then \
		shopt -s nullglob; \
		candidates=("$(SRC_DIR)"/*.ino "$(SRC_DIR)"/*.c "$(SRC_DIR)"/*.cpp "$(SRC_DIR)"/*.h); \
		if [ "$${#candidates[@]}" -eq 0 ]; then echo "ERROR: no source files found in $(SRC_DIR)/."; exit 1; fi; \
		while IFS= read -r f; do files+=("$$f"); done < <(printf '%s\n' "$${candidates[@]}" | LC_ALL=C sort); \
	else \
		candidate="$$arg"; \
		if [ ! -f "$$candidate" ]; then candidate="$(SRC_DIR)/$$(basename "$$arg")"; fi; \
		if [ ! -f "$$candidate" ]; then echo "ERROR: file not found under $(SRC_DIR)/: $$arg"; exit 1; fi; \
		resolved=$$(cd "$(SRC_DIR)" && pwd)/$$(basename "$$candidate"); \
		actual=$$(cd "$$(dirname "$$candidate")" && pwd)/$$(basename "$$candidate"); \
		if [ "$$actual" != "$$resolved" ]; then echo "ERROR: only files under $(SRC_DIR)/ can be formatted: $$arg"; exit 1; fi; \
		files=("$$candidate"); \
	fi; \
	printf '[format] Formatting %s file(s) with clang-format\n' "$${#files[@]}"; \
	clang-format -i -style=file -- "$${files[@]}"

docs:
	@set -euo pipefail; \
	command -v asciidoctor-pdf >/dev/null || { echo 'ERROR: asciidoctor-pdf is required.'; echo 'Install with: gem install asciidoctor-pdf'; exit 1; }; \
	mkdir -p build; \
	asciidoctor-pdf docs/tech_documentation/book.adoc -o build/Technical_Documentation.pdf; \
	echo "[docs] Wrote build/Technical_Documentation.pdf"
