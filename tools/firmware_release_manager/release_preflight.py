"""Preflight checks for the Firmware Release Manager.

This script preserves the release validation that used to be embedded in
the root Makefile: it checks the local Git state, validates the selected
device against source configuration, shows the release summary, and writes
the local release metadata consumed by later Makefile steps.
"""

import json
import pathlib
import re
import subprocess
import sys


def fail(message):
	"""Print a release-style error and stop the release flow."""
	print(f"\nERROR: {message}")
	print("Release process aborted.")
	sys.exit(1)


def git_output(*args):
	"""Return stdout from a Git command run in the repository root."""
	return subprocess.check_output(["git", *args], text=True).strip()


def parse_define(path, name, quoted):
	"""Read a #define value from an Arduino/C header file."""
	pattern = rf"^\s*#\s*define\s+{re.escape(name)}\s+"
	if quoted:
		pattern += r'"([^"]*)"'
	else:
		pattern += r"([^\s/]+)"
	regex = re.compile(pattern)
	for line in path.read_text().splitlines():
		match = regex.search(line)
		if match:
			return match.group(1).strip()
	fail(f"{name} was not found in {path}")


def parse_active_credential(path, name):
	"""Read an uncommented credential definition from credentials.h."""
	regex = re.compile(rf'^\s*#\s*define\s+{re.escape(name)}\s+"([^"]*)"')
	for line in path.read_text().splitlines():
		if line.lstrip().startswith("//"):
			continue
		match = regex.search(line)
		if match:
			return match.group(1)
	fail(f"{name} was not found in active credentials")


def version_parts(value):
	"""Split a version string into comparable integer chunks."""
	parts = []
	for part in re.split(r"[^\d]+", str(value)):
		if part:
			parts.append(int(part))
	return parts or [0]


def compare_versions(left, right):
	"""Compare version-ish strings using their numeric parts."""
	left_parts = version_parts(left)
	right_parts = version_parts(right)
	width = max(len(left_parts), len(right_parts))
	left_parts.extend([0] * (width - len(left_parts)))
	right_parts.extend([0] * (width - len(right_parts)))
	return (left_parts > right_parts) - (left_parts < right_parts)


def validate_inputs(manifest_path, config_path, credentials_path):
	"""Fail early if the release inputs are missing."""
	if not manifest_path.exists():
		fail(f"{manifest_path} does not exist")
	if not config_path.exists():
		fail(f"{config_path} does not exist")
	if not credentials_path.exists():
		fail(f"{credentials_path} does not exist")


def validate_git_state(credentials_path):
	"""Ensure the release starts from a clean, pushed working tree."""
	status = git_output("status", "--porcelain", "--untracked-files=normal")
	if status:
		print(status)
		fail("Git working tree is not clean")

	subprocess.run(["git", "fetch", "--quiet", "origin"], check=False)
	try:
		upstream = git_output(
			"rev-parse", "--abbrev-ref", "--symbolic-full-name", "@{u}"
		)
	except subprocess.CalledProcessError:
		fail("Current branch has no upstream. Push the branch and try again")

	local_sha = git_output("rev-parse", "HEAD")
	upstream_sha = git_output("rev-parse", upstream)
	if local_sha != upstream_sha:
		print(f"\nCurrent branch : {git_output('branch', '--show-current')}")
		print(f"Upstream branch: {upstream}")
		print(f"Local HEAD     : {local_sha}")
		print(f"Upstream HEAD  : {upstream_sha}")
		fail("Current HEAD is not pushed to GitHub. Push the branch and try again")

	tracked_credentials = subprocess.run(
		["git", "ls-files", "--error-unmatch", str(credentials_path)],
		stdout=subprocess.DEVNULL,
		stderr=subprocess.DEVNULL,
	)
	if tracked_credentials.returncode == 0:
		fail(f"{credentials_path} is tracked by Git; credentials must stay local")


def select_device(devices):
	"""Prompt the user to pick exactly one manifest device."""
	device_names = list(devices.keys())
	print("Found devices:\n")
	for index, name in enumerate(device_names, start=1):
		print(f"[{index}] {name}")

	selection = input("\nSelect exactly one device: ").strip()
	if not selection.isdigit():
		fail("Device selection must be a number")
	selection_index = int(selection)
	if selection_index < 1 or selection_index > len(device_names):
		fail("Device selection is out of range")
	return device_names[selection_index - 1]


def print_summary(
	selected_device,
	source_device,
	manifest_version,
	source_version,
	wifi_ssid,
	wifi_password,
	manifest_url,
	manifest_sha256,
	manifest_size,
	manifest_enabled,
	tag,
	asset_name,
):
	"""Show the same confirmation summary as the original Makefile flow."""
	current_branch = git_output("branch", "--show-current")

	print("\n==================================================")
	print("\nDevice\n------")
	print(f"Manifest device name   : {selected_device}")
	print(f"Source code DEVICE_NAME: {source_device}")
	print("\nVersions\n--------")
	print(f"Manifest SW number   : {manifest_version}")
	print(f"Source code SW number: {source_version}")
	print("\nWiFi\n----")
	print(f"SSID           : {wifi_ssid}")
	print(f"Password       : {wifi_password}")
	print("\nManifest\n--------")
	print(f"URL            : {manifest_url}")
	print(f"SHA-256        : {manifest_sha256}")
	print(f"Size           : {manifest_size} bytes")
	print(f"OTA enabled    : {manifest_enabled}")
	print("\nGit\n---")
	print(f"Branch         : {current_branch}")
	print("Working tree   : Clean")
	print("\nRelease\n-------")
	print(f"Tag            : {tag}")
	print(f"Firmware asset : {asset_name}")
	print("\n==================================================\n")


def main(argv):
	"""Run release preflight and write release-meta.json."""
	if len(argv) != 5:
		fail(
			"usage: release_preflight.py "
			"<manifest> <config> <credentials> <release-meta>"
		)

	manifest_path = pathlib.Path(argv[1])
	config_path = pathlib.Path(argv[2])
	credentials_path = pathlib.Path(argv[3])
	meta_path = pathlib.Path(argv[4])

	validate_inputs(manifest_path, config_path, credentials_path)
	validate_git_state(credentials_path)

	manifest = json.loads(manifest_path.read_text())
	devices = manifest.get("devices")
	if not isinstance(devices, dict) or not devices:
		fail("manifest does not contain any devices")

	selected_device = select_device(devices)
	device_entry = devices[selected_device]
	source_device = parse_define(config_path, "DEVICE_NAME", quoted=True)
	source_version = parse_define(config_path, "SOFTWARE_VERSION", quoted=False)
	wifi_ssid = parse_active_credential(credentials_path, "WIFI_SSID")
	wifi_password = parse_active_credential(credentials_path, "WIFI_PASSWORD")
	manifest_version = str(device_entry.get("version", ""))
	manifest_url = str(device_entry.get("url", ""))
	manifest_sha256 = str(device_entry.get("sha256", ""))
	manifest_size = device_entry.get("size", "")
	manifest_enabled = device_entry.get("enabled", "")

	if selected_device != source_device:
		print(f"\nSelected device               : {selected_device}")
		print(f"DEVICE_NAME in the source code: {source_device}")
		fail("Device mismatch. Change DEVICE_NAME in config.h and try again")

	if compare_versions(source_version, manifest_version) <= 0:
		print(
			"\nWARNING: Source firmware version is not newer than "
			"the published manifest version."
		)

	tag = f"v{source_version}"
	asset_device_name = source_device.replace(" ", "_")
	asset_name = f"firmware_{asset_device_name}_v.{source_version}.bin"

	print_summary(
		selected_device,
		source_device,
		manifest_version,
		source_version,
		wifi_ssid,
		wifi_password,
		manifest_url,
		manifest_sha256,
		manifest_size,
		manifest_enabled,
		tag,
		asset_name,
	)

	answer = input("Continue? [y/N] ").strip().lower()
	if answer not in {"y", "yes"}:
		fail("User declined release confirmation")

	meta_path.parent.mkdir(parents=True, exist_ok=True)
	meta_path.write_text(
		json.dumps(
			{
				"DEVICE_NAME": source_device,
				"SOFTWARE_VERSION": source_version,
				"MANIFEST_VERSION": manifest_version,
				"FIRMWARE_ASSET": asset_name,
				"RELEASE_TAG": tag,
			},
			indent=2,
		)
		+ "\n"
	)
	print(f"\n[release] Wrote local metadata to {meta_path}")


if __name__ == "__main__":
	main(sys.argv)
