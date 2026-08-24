"""Update the OTA manifest from a Firmware Release Manager contract.

The script updates only the selected device block in ota/manifest.json.
It preserves the original manifest shape and explicitly checks that the
device set and the `enabled` flag are not changed by the update.
"""

import json
import pathlib
import re
import sys
import urllib.parse


def fail(message):
	"""Print an error and stop manifest publication."""
	print(f"ERROR: {message}", file=sys.stderr)
	sys.exit(1)


def find_matching_brace(text, start_index):
	"""Find the closing brace for a JSON object inside raw manifest text."""
	depth = 0
	in_string = False
	escaped = False
	for index in range(start_index, len(text)):
		char = text[index]
		if in_string:
			if escaped:
				escaped = False
			elif char == "\\":
				escaped = True
			elif char == '"':
				in_string = False
			continue
		if char == '"':
			in_string = True
		elif char == "{":
			depth += 1
		elif char == "}":
			depth -= 1
			if depth == 0:
				return index
	fail("could not find matching manifest device block brace")


def json_value(value):
	"""Serialize one manifest field value."""
	return json.dumps(value, ensure_ascii=False)


def replace_field(block, field, value):
	"""Replace exactly one simple JSON field inside the selected block."""
	pattern = re.compile(
		rf'("{re.escape(field)}"\s*:\s*)'
		r'(?:"(?:\\.|[^"\\])*"|\d+|true|false|null)'
	)
	updated, count = pattern.subn(
		lambda match: match.group(1) + json_value(value), block, count=1
	)
	if count != 1:
		fail(f"field {field} was not found exactly once in selected manifest block")
	return updated


def release_url(repo, release_tag, asset_name):
	"""Build the GitHub Release asset URL used by OTA clients."""
	return (
		f"https://github.com/{repo}/releases/download/"
		f"{urllib.parse.quote(str(release_tag), safe='')}/"
		f"{urllib.parse.quote(str(asset_name), safe='')}"
	)


def load_release_info(release_info_path, release_tag_arg):
	"""Read and validate release-info.json produced by the release flow."""
	info = json.loads(release_info_path.read_text())
	device = info.get("DEVICE_NAME")
	version = info.get("SOFTWARE_VERSION")
	sha256 = info.get("SHA-256")
	size = info.get("SIZE")
	asset_name = info.get("FIRMWARE_ASSET")
	release_tag = release_tag_arg or info.get("RELEASE_TAG")

	if not all([device, version, sha256, size, asset_name, release_tag]):
		fail("release-info.json is missing required metadata")

	return device, version, sha256, size, asset_name, release_tag


def update_manifest(manifest_path, release_info_path, release_tag_arg, repo):
	"""Apply a release-info.json update to one manifest device entry."""
	device, version, sha256, size, asset_name, release_tag = load_release_info(
		release_info_path, release_tag_arg
	)
	firmware_url = release_url(repo, release_tag, asset_name)

	text = manifest_path.read_text()
	manifest_before = json.loads(text)
	devices = manifest_before.get("devices", {})
	if device not in devices:
		fail(f"device {device!r} was not found in manifest")
	enabled_before = devices[device].get("enabled")

	device_match = re.search(rf'"{re.escape(device)}"\s*:\s*\{{', text)
	if not device_match:
		fail(f"device block for {device!r} was not found in manifest text")

	block_start = text.find("{", device_match.start())
	block_end = find_matching_brace(text, block_start)
	block = text[block_start:block_end + 1]
	block = replace_field(block, "version", str(version))
	block = replace_field(block, "url", firmware_url)
	block = replace_field(block, "sha256", str(sha256))
	block = replace_field(block, "size", int(size))
	updated_text = text[:block_start] + block + text[block_end + 1:]

	manifest_after = json.loads(updated_text)
	enabled_after = manifest_after["devices"][device].get("enabled")
	if enabled_before != enabled_after:
		fail("manifest enabled field changed unexpectedly")
	if set(manifest_before.get("devices", {}).keys()) != set(
		manifest_after.get("devices", {}).keys()
	):
		fail("manifest device set changed unexpectedly")

	manifest_path.write_text(updated_text)
	print(f"Updated {manifest_path} for {device} {version}")
	print(f"Firmware URL: {firmware_url}")


def main(argv):
	"""Parse Makefile-provided arguments and update the manifest."""
	if len(argv) != 5:
		fail(
			"usage: publish_manifest.py "
			"<manifest> <release-info> <release-tag> <repo>"
		)

	manifest_path = pathlib.Path(argv[1])
	release_info_path = pathlib.Path(argv[2])
	release_tag_arg = argv[3].strip()
	repo = argv[4].strip()

	update_manifest(manifest_path, release_info_path, release_tag_arg, repo)


if __name__ == "__main__":
	main(sys.argv)
