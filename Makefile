
##################################################################
# Firmware Release Manager for the 42 Smart Cluster Sign Project
#
# Have you just finished working on a new firmware for the Sign
# and now want to upload it to the device? This tool automates
# the whole process. Just call 'make release' in the Terminal,
# fix the issues that the tool points out, if any, and the rest
# will be taken care for you.
#
# If you want to speed up the process, send the '/ota' command
# in the Telegram chat or press the OTA button on the physical
# device when you are done with this Firmware Release Manager.
##################################################################


SHELL := /bin/bash

REPO ?= RomanAlexandroff/42-Smart-Cluster-Sign
MANIFEST ?= ota/manifest.json
CONFIG ?= src/config.h
CREDENTIALS ?= src/credentials.h
SKETCH ?= src/src.ino
LIBRARIES_PATH ?= libraries
ARDUINO_BOARD ?= esp32:esp32:XIAO_ESP32C3:UploadSpeed=230400,CDCOnBoot=default,CPUFreq=160,FlashFreq=80,FlashMode=qio,FlashSize=4M,PartitionScheme=min_spiffs,DebugLevel=debug,EraseFlash=none
ARDUINO_CLI_CONFIG ?= tools/arduino-cli/arduino-cli.yaml
RELEASE_DIR ?= build
RELEASE_META ?= $(RELEASE_DIR)/release-meta.json
RELEASE_INFO ?= $(RELEASE_DIR)/release-info.json

.PHONY: help release publish-manifest

help:
	@printf '%s\n' \
		'Available targets:' \
		'  make release           Run the Firmware Release Manager.' \
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
	@set -euo pipefail; RELEASE_PREFLIGHT=$$(awk '/^# BEGIN RELEASE_PREFLIGHT_PY/{flag=1;next}/^# END RELEASE_PREFLIGHT_PY/{flag=0}flag{sub(/^# ?/,"");print}' Makefile); python3 -c "$$RELEASE_PREFLIGHT" "$(MANIFEST)" "$(CONFIG)" "$(CREDENTIALS)" "$(RELEASE_META)"
	@set -euo pipefail; \
	TAG=$$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["RELEASE_TAG"])' "$(RELEASE_META)"); \
	LOCAL_TAG_EXISTS=0; REMOTE_TAG_EXISTS=0; RELEASE_EXISTS=0; \
	if git rev-parse -q --verify "refs/tags/$$TAG" >/dev/null; then LOCAL_TAG_EXISTS=1; fi; \
	if git ls-remote --exit-code --tags origin "refs/tags/$$TAG" >/dev/null 2>&1; then REMOTE_TAG_EXISTS=1; fi; \
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
		if [ "$$LOCAL_TAG_EXISTS" = "1" ]; then git tag -d "$$TAG"; fi; \
		if [ "$$REMOTE_TAG_EXISTS" = "1" ]; then git push origin ":refs/tags/$$TAG"; fi; \
	fi
	@set -euo pipefail; mkdir -p "$(RELEASE_DIR)"; printf '\n[release] Compiling firmware locally\n\n'; arduino-cli --config-file "$(ARDUINO_CLI_CONFIG)" compile --fqbn "$(ARDUINO_BOARD)" --libraries "$(LIBRARIES_PATH)" --output-dir "$(RELEASE_DIR)" --build-property build.partitions=min_spiffs --build-property upload.maximum_size=1966080 --verbose "$(SKETCH)"
	@set -euo pipefail; FIRMWARE_SOURCE=$$(python3 -c 'import pathlib,sys; root=pathlib.Path(sys.argv[1]); firmware_name=pathlib.Path(sys.argv[2]).name + ".bin"; matches=list(root.glob(firmware_name)); sys.exit(f"ERROR: expected firmware binary {firmware_name} was not found in {root}") if len(matches) != 1 else print(matches[0])' "$(RELEASE_DIR)" "$(SKETCH)"); FIRMWARE_ASSET=$$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["FIRMWARE_ASSET"])' "$(RELEASE_META)"); cp "$$FIRMWARE_SOURCE" "$(RELEASE_DIR)/$$FIRMWARE_ASSET"
	@set -euo pipefail; FIRMWARE_ASSET=$$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["FIRMWARE_ASSET"])' "$(RELEASE_META)"); FIRMWARE_ASSET_PATH="$(RELEASE_DIR)/$$FIRMWARE_ASSET"; SHA256=$$(python3 -c 'import hashlib,sys; h=hashlib.sha256(); f=open(sys.argv[1],"rb"); [h.update(c) for c in iter(lambda:f.read(1048576), b"")]; print(h.hexdigest())' "$$FIRMWARE_ASSET_PATH"); SIZE=$$(python3 -c 'import pathlib,sys; print(pathlib.Path(sys.argv[1]).stat().st_size)' "$$FIRMWARE_ASSET_PATH"); python3 -c 'import json,pathlib,sys; info=json.load(open(sys.argv[1])); info["SHA-256"]=sys.argv[3]; info["SIZE"]=int(sys.argv[4]); pathlib.Path(sys.argv[2]).write_text(json.dumps(info, indent=2)+"\n"); print(f"[release] Wrote release contract to {sys.argv[2]}")' "$(RELEASE_META)" "$(RELEASE_INFO)" "$$SHA256" "$$SIZE"
	@set -euo pipefail; TAG=$$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["RELEASE_TAG"])' "$(RELEASE_META)"); DEVICE_NAME=$$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["DEVICE_NAME"])' "$(RELEASE_META)"); SOFTWARE_VERSION=$$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["SOFTWARE_VERSION"])' "$(RELEASE_META)"); FIRMWARE_ASSET=$$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["FIRMWARE_ASSET"])' "$(RELEASE_META)"); printf '\n[release] Creating GitHub Release %s\n\n' "$$TAG"; gh release create "$$TAG" "$(RELEASE_DIR)/$$FIRMWARE_ASSET" "$(RELEASE_INFO)" --repo "$(REPO)" --target "$$(git rev-parse HEAD)" --title "$$DEVICE_NAME $$SOFTWARE_VERSION" --notes "Automated OTA firmware release for $$DEVICE_NAME, version $$SOFTWARE_VERSION."
	@set -euo pipefail; rm -f "$(RELEASE_INFO)"; printf '\n[release] Release complete. Local release-info.json was removed after upload.\n'

publish-manifest:
	@set -euo pipefail; command -v python3 >/dev/null || { echo 'ERROR: python3 is required.'; exit 1; }
	@set -euo pipefail; test -n "$(RELEASE_INFO)" || { echo 'ERROR: RELEASE_INFO is required.'; exit 1; }
	@set -euo pipefail; test -f "$(RELEASE_INFO)" || { echo "ERROR: $(RELEASE_INFO) does not exist."; exit 1; }
	@set -euo pipefail; awk '/^# BEGIN PUBLISH_MANIFEST_PY/{flag=1;next}/^# END PUBLISH_MANIFEST_PY/{flag=0}flag{sub(/^# ?/,"");print}' Makefile | python3 - "$(MANIFEST)" "$(RELEASE_INFO)" "$(RELEASE_TAG)" "$(REPO)"

# BEGIN RELEASE_PREFLIGHT_PY
# import json
# import pathlib
# import re
# import subprocess
# import sys
#
# manifest_path = pathlib.Path(sys.argv[1])
# config_path = pathlib.Path(sys.argv[2])
# credentials_path = pathlib.Path(sys.argv[3])
# meta_path = pathlib.Path(sys.argv[4])
#
# def fail(message):
#     print(f"\nERROR: {message}")
#     print("Release process aborted.")
#     sys.exit(1)
#
# def git_output(*args):
#     return subprocess.check_output(["git", *args], text=True).strip()
#
# def parse_define(path, name, quoted):
#     pattern = rf"^\s*#\s*define\s+{re.escape(name)}\s+"
#     if quoted:
#         pattern += r'"([^"]*)"'
#     else:
#         pattern += r'([^\s/]+)'
#     regex = re.compile(pattern)
#     for line in path.read_text().splitlines():
#         match = regex.search(line)
#         if match:
#             return match.group(1).strip()
#     fail(f"{name} was not found in {path}")
#
# def parse_active_credential(path, name):
#     regex = re.compile(rf"^\s*#\s*define\s+{re.escape(name)}\s+\"([^\"]*)\"")
#     for line in path.read_text().splitlines():
#         if line.lstrip().startswith("//"):
#             continue
#         match = regex.search(line)
#         if match:
#             return match.group(1)
#     fail(f"{name} was not found in active credentials")
#
# def version_parts(value):
#     parts = []
#     for part in re.split(r"[^\d]+", str(value)):
#         if part:
#             parts.append(int(part))
#     return parts or [0]
#
# def compare_versions(left, right):
#     left_parts = version_parts(left)
#     right_parts = version_parts(right)
#     width = max(len(left_parts), len(right_parts))
#     left_parts.extend([0] * (width - len(left_parts)))
#     right_parts.extend([0] * (width - len(right_parts)))
#     return (left_parts > right_parts) - (left_parts < right_parts)
#
# if not manifest_path.exists():
#     fail(f"{manifest_path} does not exist")
# if not config_path.exists():
#     fail(f"{config_path} does not exist")
# if not credentials_path.exists():
#     fail(f"{credentials_path} does not exist")
#
# status = git_output("status", "--porcelain", "--untracked-files=normal")
# if status:
#     print(status)
#     fail("Git working tree is not clean")
#
# subprocess.run(["git", "fetch", "--quiet", "origin"], check=False)
# try:
#     upstream = git_output("rev-parse", "--abbrev-ref", "--symbolic-full-name", "@{u}")
# except subprocess.CalledProcessError:
#     fail("Current branch has no upstream. Push the branch and try again")
# local_sha = git_output("rev-parse", "HEAD")
# upstream_sha = git_output("rev-parse", upstream)
# if local_sha != upstream_sha:
#     print(f"\nCurrent branch : {git_output('branch', '--show-current')}")
#     print(f"Upstream branch: {upstream}")
#     print(f"Local HEAD     : {local_sha}")
#     print(f"Upstream HEAD  : {upstream_sha}")
#     fail("Current HEAD is not pushed to GitHub. Push the branch and try again")
#
# tracked_credentials = subprocess.run(
#     ["git", "ls-files", "--error-unmatch", str(credentials_path)],
#     stdout=subprocess.DEVNULL,
#     stderr=subprocess.DEVNULL,
# )
# if tracked_credentials.returncode == 0:
#     fail(f"{credentials_path} is tracked by Git; credentials must stay local")
#
# manifest = json.loads(manifest_path.read_text())
# devices = manifest.get("devices")
# if not isinstance(devices, dict) or not devices:
#     fail("manifest does not contain any devices")
#
# device_names = list(devices.keys())
# print("Found devices:\n")
# for index, name in enumerate(device_names, start=1):
#     print(f"[{index}] {name}")
#
# selection = input("\nSelect exactly one device: ").strip()
# if not selection.isdigit():
#     fail("Device selection must be a number")
# selection_index = int(selection)
# if selection_index < 1 or selection_index > len(device_names):
#     fail("Device selection is out of range")
#
# selected_device = device_names[selection_index - 1]
# device_entry = devices[selected_device]
# source_device = parse_define(config_path, "DEVICE_NAME", quoted=True)
# source_version = parse_define(config_path, "SOFTWARE_VERSION", quoted=False)
# wifi_ssid = parse_active_credential(credentials_path, "WIFI_SSID")
# wifi_password = parse_active_credential(credentials_path, "WIFI_PASSWORD")
# manifest_version = str(device_entry.get("version", ""))
# manifest_url = str(device_entry.get("url", ""))
# manifest_sha256 = str(device_entry.get("sha256", ""))
# manifest_size = device_entry.get("size", "")
# manifest_enabled = device_entry.get("enabled", "")
#
# if selected_device != source_device:
#     print(f"\nSelected device               : {selected_device}")
#     print(f"DEVICE_NAME in the source code: {source_device}")
#     fail("Device mismatch. Change DEVICE_NAME in config.h and try again")
#
# if compare_versions(source_version, manifest_version) <= 0:
#     print("\nWARNING: Source firmware version is not newer than the published manifest version.")
#
# tag = f"v{source_version}"
# asset_device_name = source_device.replace(" ", "_")
# asset_name = f"firmware_{asset_device_name}_v.{source_version}.bin"
# current_branch = git_output("branch", "--show-current")
#
# print("\n==================================================")
# print("\nDevice\n------")
# print(f"Manifest device name   : {selected_device}")
# print(f"Source code DEVICE_NAME: {source_device}")
# print("\nVersions\n--------")
# print(f"Manifest SW number   : {manifest_version}")
# print(f"Source code SW number: {source_version}")
# print("\nWiFi\n----")
# print(f"SSID           : {wifi_ssid}")
# print(f"Password       : {wifi_password}")
# print("\nManifest\n--------")
# print(f"URL            : {manifest_url}")
# print(f"SHA-256        : {manifest_sha256}")
# print(f"Size           : {manifest_size} bytes")
# print(f"OTA enabled    : {manifest_enabled}")
# print("\nGit\n---")
# print(f"Branch         : {current_branch}")
# print("Working tree   : Clean")
# print("\nRelease\n-------")
# print(f"Tag            : {tag}")
# print(f"Firmware asset : {asset_name}")
# print("\n==================================================\n")
#
# answer = input("Continue? [y/N] ").strip().lower()
# if answer not in {"y", "yes"}:
#     fail("User declined release confirmation")
#
# meta_path.parent.mkdir(parents=True, exist_ok=True)
# meta_path.write_text(json.dumps({
#     "DEVICE_NAME": source_device,
#     "SOFTWARE_VERSION": source_version,
#     "MANIFEST_VERSION": manifest_version,
#     "FIRMWARE_ASSET": asset_name,
#     "RELEASE_TAG": tag,
# }, indent=2) + "\n")
# print(f"\n[release] Wrote local metadata to {meta_path}")
# END RELEASE_PREFLIGHT_PY

# BEGIN PUBLISH_MANIFEST_PY
# import json
# import pathlib
# import re
# import sys
# import urllib.parse
#
# manifest_path = pathlib.Path(sys.argv[1])
# release_info_path = pathlib.Path(sys.argv[2])
# release_tag_arg = sys.argv[3].strip()
# repo = sys.argv[4].strip()
#
# def fail(message):
#     print(f"ERROR: {message}", file=sys.stderr)
#     sys.exit(1)
#
# def find_matching_brace(text, start_index):
#     depth = 0
#     in_string = False
#     escaped = False
#     for index in range(start_index, len(text)):
#         char = text[index]
#         if in_string:
#             if escaped:
#                 escaped = False
#             elif char == "\\":
#                 escaped = True
#             elif char == '"':
#                 in_string = False
#             continue
#         if char == '"':
#             in_string = True
#         elif char == "{":
#             depth += 1
#         elif char == "}":
#             depth -= 1
#             if depth == 0:
#                 return index
#     fail("could not find matching manifest device block brace")
#
# def json_value(value):
#     return json.dumps(value, ensure_ascii=False)
#
# def replace_field(block, field, value):
#     pattern = re.compile(
#         rf'("{re.escape(field)}"\s*:\s*)(?:"(?:\\.|[^"\\])*"|\d+|true|false|null)'
#     )
#     updated, count = pattern.subn(lambda match: match.group(1) + json_value(value), block, count=1)
#     if count != 1:
#         fail(f"field {field} was not found exactly once in selected manifest block")
#     return updated
#
# info = json.loads(release_info_path.read_text())
# device = info.get("DEVICE_NAME")
# version = info.get("SOFTWARE_VERSION")
# sha256 = info.get("SHA-256")
# size = info.get("SIZE")
# asset_name = info.get("FIRMWARE_ASSET")
# release_tag = release_tag_arg or info.get("RELEASE_TAG")
#
# if not all([device, version, sha256, size, asset_name, release_tag]):
#     fail("release-info.json is missing required metadata")
#
# firmware_url = (
#     f"https://github.com/{repo}/releases/download/"
#     f"{urllib.parse.quote(str(release_tag), safe='')}/"
#     f"{urllib.parse.quote(str(asset_name), safe='')}"
# )
#
# text = manifest_path.read_text()
# manifest_before = json.loads(text)
# devices = manifest_before.get("devices", {})
# if device not in devices:
#     fail(f"device {device!r} was not found in manifest")
# enabled_before = devices[device].get("enabled")
#
# device_match = re.search(rf'"{re.escape(device)}"\s*:\s*\{{', text)
# if not device_match:
#     fail(f"device block for {device!r} was not found in manifest text")
# block_start = text.find("{", device_match.start())
# block_end = find_matching_brace(text, block_start)
# block = text[block_start:block_end + 1]
# block = replace_field(block, "version", str(version))
# block = replace_field(block, "url", firmware_url)
# block = replace_field(block, "sha256", str(sha256))
# block = replace_field(block, "size", int(size))
# updated_text = text[:block_start] + block + text[block_end + 1:]
#
# manifest_after = json.loads(updated_text)
# enabled_after = manifest_after["devices"][device].get("enabled")
# if enabled_before != enabled_after:
#     fail("manifest enabled field changed unexpectedly")
# if set(manifest_before.get("devices", {}).keys()) != set(manifest_after.get("devices", {}).keys()):
#     fail("manifest device set changed unexpectedly")
#
# manifest_path.write_text(updated_text)
# print(f"Updated {manifest_path} for {device} {version}")
# print(f"Firmware URL: {firmware_url}")
# END PUBLISH_MANIFEST_PY
