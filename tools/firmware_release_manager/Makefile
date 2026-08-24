##################################################################
# Firmware Release Manager for the 42 Smart Cluster Sign Project
#
# This Makefile owns the release-manager internals. It is normally
# called by the root Makefile, but it can also be run directly from
# this folder because all project paths are resolved from ROOT_DIR.
#
# If you want to speed up the OTA flow, send the '/ota' command in
# the Telegram chat or press the OTA button on the physical device
# when this Firmware Release Manager is done.
##################################################################

SHELL := /bin/bash

ROOT_DIR ?= $(abspath ../..)
SCRIPT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

REPO ?= RomanAlexandroff/42-Smart-Cluster-Sign
MANIFEST ?= $(ROOT_DIR)/ota/manifest.json
CONFIG ?= $(ROOT_DIR)/src/config.h
CREDENTIALS ?= $(ROOT_DIR)/src/credentials.h
SKETCH ?= $(ROOT_DIR)/src/src.ino
LIBRARIES_PATH ?= $(ROOT_DIR)/libraries
ARDUINO_BOARD ?= esp32:esp32:XIAO_ESP32C3:UploadSpeed=230400,CDCOnBoot=default,CPUFreq=160,FlashFreq=80,FlashMode=qio,FlashSize=4M,PartitionScheme=min_spiffs,DebugLevel=debug,EraseFlash=none
ARDUINO_CLI_CONFIG ?= $(ROOT_DIR)/tools/arduino-cli/arduino-cli.yaml
RELEASE_DIR ?= $(ROOT_DIR)/build
RELEASE_META ?= $(RELEASE_DIR)/release-meta.json
RELEASE_INFO ?= $(RELEASE_DIR)/release-info.json

RELEASE_PREFLIGHT := $(SCRIPT_DIR)/release_preflight.py
RELEASE_INFO_TOOL := $(SCRIPT_DIR)/release_info.py
PUBLISH_MANIFEST_TOOL := $(SCRIPT_DIR)/publish_manifest.py

.PHONY: help release publish-manifest

help:
	@printf '%s\n' \
		'Firmware Release Manager targets:' \
		'  make release           Build and publish an OTA firmware release.' \
		'  make publish-manifest  Update ota/manifest.json from release-info.json.'

release:
	@set -euo pipefail; printf '\n[release] Starting OTA release manager\n\n'
	@set -euo pipefail; command -v git >/dev/null || { echo 'ERROR: git is required.'; exit 1; }
	@set -euo pipefail; command -v gh >/dev/null || { echo 'ERROR: GitHub CLI (gh) is required.'; exit 1; }
	@set -euo pipefail; gh auth status -h github.com 2>&1 | grep -q workflow || { echo 'ERROR: GitHub CLI token is missing the workflow scope.'; echo 'Run: gh auth refresh -h github.com -s workflow'; exit 1; }
	@set -euo pipefail; command -v arduino-cli >/dev/null || { echo 'ERROR: arduino-cli is required.'; exit 1; }
	@set -euo pipefail; command -v python3 >/dev/null || { echo 'ERROR: python3 is required.'; exit 1; }
	@set -euo pipefail; test -f "$(ARDUINO_CLI_CONFIG)" || { echo "ERROR: $(ARDUINO_CLI_CONFIG) does not exist."; exit 1; }
	@set -euo pipefail; rm -rf "$(RELEASE_DIR)/ota-release"; rm -f "$(RELEASE_DIR)"/*.bin "$(RELEASE_META)" "$(RELEASE_INFO)"
	@set -euo pipefail; cd "$(ROOT_DIR)" && python3 "$(RELEASE_PREFLIGHT)" "$(MANIFEST)" "$(CONFIG)" "$(CREDENTIALS)" "$(RELEASE_META)"
	@set -euo pipefail; \
	TAG=$$(python3 "$(RELEASE_INFO_TOOL)" read-meta "$(RELEASE_META)" RELEASE_TAG); \
	LOCAL_TAG_EXISTS=0; REMOTE_TAG_EXISTS=0; RELEASE_EXISTS=0; \
	if git -C "$(ROOT_DIR)" rev-parse -q --verify "refs/tags/$$TAG" >/dev/null; then LOCAL_TAG_EXISTS=1; fi; \
	if git -C "$(ROOT_DIR)" ls-remote --exit-code --tags origin "refs/tags/$$TAG" >/dev/null 2>&1; then REMOTE_TAG_EXISTS=1; fi; \
	if gh release view "$$TAG" --repo "$(REPO)" >/dev/null 2>&1; then RELEASE_EXISTS=1; fi; \
	if [ "$$LOCAL_TAG_EXISTS" = "1" ] || [ "$$REMOTE_TAG_EXISTS" = "1" ] || [ "$$RELEASE_EXISTS" = "1" ]; then \
		echo; \
		echo "Release identity $$TAG already exists:"; \
		if [ "$$LOCAL_TAG_EXISTS" = "1" ]; then echo "  - local Git tag"; fi; \
		if [ "$$REMOTE_TAG_EXISTS" = "1" ]; then echo "  - remote Git tag"; fi; \
		if [ "$$RELEASE_EXISTS" = "1" ]; then echo "  - GitHub Release"; fi; \
		echo; \
		printf "Delete the existing release identity and reuse $$TAG? [y/N] "; \
		read -r answer; \
		case "$$answer" in \
			y|Y|yes|YES) ;; \
			*) echo "Release process aborted."; exit 1 ;; \
		esac; \
		if [ "$$RELEASE_EXISTS" = "1" ]; then gh release delete "$$TAG" --repo "$(REPO)" --yes; fi; \
		if [ "$$LOCAL_TAG_EXISTS" = "1" ]; then git -C "$(ROOT_DIR)" tag -d "$$TAG"; fi; \
		if [ "$$REMOTE_TAG_EXISTS" = "1" ]; then git -C "$(ROOT_DIR)" push origin ":refs/tags/$$TAG"; fi; \
	fi
	@set -euo pipefail; mkdir -p "$(RELEASE_DIR)"; printf '\n[release] Compiling firmware locally\n\n'; arduino-cli --config-file "$(ARDUINO_CLI_CONFIG)" compile --fqbn "$(ARDUINO_BOARD)" --libraries "$(LIBRARIES_PATH)" --output-dir "$(RELEASE_DIR)" --build-property build.partitions=min_spiffs --build-property upload.maximum_size=1966080 --verbose "$(SKETCH)"
	@set -euo pipefail; FIRMWARE_SOURCE=$$(python3 "$(RELEASE_INFO_TOOL)" find-firmware "$(RELEASE_DIR)" "$(SKETCH)"); FIRMWARE_ASSET=$$(python3 "$(RELEASE_INFO_TOOL)" read-meta "$(RELEASE_META)" FIRMWARE_ASSET); cp "$$FIRMWARE_SOURCE" "$(RELEASE_DIR)/$$FIRMWARE_ASSET"
	@set -euo pipefail; FIRMWARE_ASSET=$$(python3 "$(RELEASE_INFO_TOOL)" read-meta "$(RELEASE_META)" FIRMWARE_ASSET); FIRMWARE_ASSET_PATH="$(RELEASE_DIR)/$$FIRMWARE_ASSET"; python3 "$(RELEASE_INFO_TOOL)" write-release-info "$(RELEASE_META)" "$(RELEASE_INFO)" "$$FIRMWARE_ASSET_PATH"
	@set -euo pipefail; TAG=$$(python3 "$(RELEASE_INFO_TOOL)" read-meta "$(RELEASE_META)" RELEASE_TAG); DEVICE_NAME=$$(python3 "$(RELEASE_INFO_TOOL)" read-meta "$(RELEASE_META)" DEVICE_NAME); SOFTWARE_VERSION=$$(python3 "$(RELEASE_INFO_TOOL)" read-meta "$(RELEASE_META)" SOFTWARE_VERSION); FIRMWARE_ASSET=$$(python3 "$(RELEASE_INFO_TOOL)" read-meta "$(RELEASE_META)" FIRMWARE_ASSET); printf '\n[release] Creating GitHub Release %s\n\n' "$$TAG"; gh release create "$$TAG" "$(RELEASE_DIR)/$$FIRMWARE_ASSET" "$(RELEASE_INFO)" --repo "$(REPO)" --target "$$(git -C "$(ROOT_DIR)" rev-parse HEAD)" --title "$$DEVICE_NAME $$SOFTWARE_VERSION" --notes "Automated OTA firmware release for $$DEVICE_NAME, version $$SOFTWARE_VERSION."
	@set -euo pipefail; rm -f "$(RELEASE_INFO)"; printf '\n[release] Release complete. Local release-info.json was removed after upload.\n'

publish-manifest:
	@set -euo pipefail; command -v python3 >/dev/null || { echo 'ERROR: python3 is required.'; exit 1; }
	@set -euo pipefail; test -n "$(RELEASE_INFO)" || { echo 'ERROR: RELEASE_INFO is required.'; exit 1; }
	@set -euo pipefail; test -f "$(RELEASE_INFO)" || { echo "ERROR: $(RELEASE_INFO) does not exist."; exit 1; }
	@set -euo pipefail; python3 "$(PUBLISH_MANIFEST_TOOL)" "$(MANIFEST)" "$(RELEASE_INFO)" "$(RELEASE_TAG)" "$(REPO)"
