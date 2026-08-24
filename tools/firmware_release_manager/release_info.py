"""Release metadata helpers for the Firmware Release Manager.

The Makefile uses this small CLI for operations that were previously
implemented as inline `python3 -c` snippets: reading release metadata,
finding the compiled firmware binary, and writing the upload contract.
"""

import argparse
import hashlib
import json
import pathlib
import sys


def fail(message):
	"""Print an error and return a non-zero exit code."""
	print(f"ERROR: {message}", file=sys.stderr)
	sys.exit(1)


def load_json(path):
	"""Load JSON from a path with a consistent error message."""
	try:
		return json.loads(path.read_text())
	except FileNotFoundError:
		fail(f"{path} does not exist")
	except json.JSONDecodeError as error:
		fail(f"{path} is not valid JSON: {error}")


def read_meta(args):
	"""Print one key from release-meta.json."""
	meta_path = pathlib.Path(args.meta)
	info = load_json(meta_path)
	try:
		print(info[args.key])
	except KeyError:
		fail(f"{args.key} was not found in {meta_path}")


def find_firmware(args):
	"""Find the firmware binary emitted by arduino-cli for the sketch."""
	release_dir = pathlib.Path(args.release_dir)
	firmware_name = pathlib.Path(args.sketch).name + ".bin"
	matches = list(release_dir.glob(firmware_name))
	if len(matches) != 1:
		fail(f"expected firmware binary {firmware_name} was not found in {release_dir}")
	print(matches[0])


def sha256_file(path):
	"""Calculate a firmware file SHA-256 without loading it all at once."""
	hash_object = hashlib.sha256()
	with path.open("rb") as file:
		for chunk in iter(lambda: file.read(1048576), b""):
			hash_object.update(chunk)
	return hash_object.hexdigest()


def write_release_info(args):
	"""Write release-info.json with firmware checksum and size."""
	meta_path = pathlib.Path(args.meta)
	output_path = pathlib.Path(args.output)
	firmware_path = pathlib.Path(args.firmware)

	info = load_json(meta_path)
	if not firmware_path.exists():
		fail(f"{firmware_path} does not exist")

	info["SHA-256"] = sha256_file(firmware_path)
	info["SIZE"] = firmware_path.stat().st_size
	output_path.write_text(json.dumps(info, indent=2) + "\n")
	print(f"[release] Wrote release contract to {output_path}")


def build_parser():
	"""Create the CLI parser used by the Makefile."""
	parser = argparse.ArgumentParser(
		description="Firmware Release Manager metadata helper."
	)
	subparsers = parser.add_subparsers(dest="command", required=True)

	read_parser = subparsers.add_parser(
		"read-meta", help="print a key from release-meta.json"
	)
	read_parser.add_argument("meta")
	read_parser.add_argument("key")
	read_parser.set_defaults(func=read_meta)

	find_parser = subparsers.add_parser(
		"find-firmware", help="locate the compiled firmware binary"
	)
	find_parser.add_argument("release_dir")
	find_parser.add_argument("sketch")
	find_parser.set_defaults(func=find_firmware)

	write_parser = subparsers.add_parser(
		"write-release-info", help="write release-info.json"
	)
	write_parser.add_argument("meta")
	write_parser.add_argument("output")
	write_parser.add_argument("firmware")
	write_parser.set_defaults(func=write_release_info)

	return parser


def main():
	"""Dispatch one release-info helper command."""
	parser = build_parser()
	args = parser.parse_args()
	args.func(args)


if __name__ == "__main__":
	main()
