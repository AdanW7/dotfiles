#!/usr/bin/env python3

import os
import subprocess
import sys


def normalize_path(path: str) -> str:
    return path.replace("\\", "/")


def main() -> int:
    if len(sys.argv) < 6:
        print("Expected git external diff arguments", file=sys.stderr)
        return 1

    display_path = sys.argv[1]
    old_path = normalize_path(sys.argv[2])
    new_path = normalize_path(sys.argv[5])

    diff_cmd = ["git", "diff", "--no-index", "--no-ext-diff", old_path, new_path]
    diff_proc = subprocess.run(diff_cmd, capture_output=True, text=True)

    # git diff returns 1 when differences are found, which is the normal case here.
    if diff_proc.returncode not in (0, 1):
        sys.stderr.write(diff_proc.stderr)
        return diff_proc.returncode

    diff_text = diff_proc.stdout.replace(old_path, display_path).replace(new_path, display_path)

    delta_cmd = ["delta", "--dark", "--paging=never"]
    if columns := os.environ.get("LAZYGIT_COLUMNS"):
        delta_cmd.append(f"--width={columns}")

    delta_proc = subprocess.run(delta_cmd, input=diff_text, text=True)
    return delta_proc.returncode


if __name__ == "__main__":
    raise SystemExit(main())