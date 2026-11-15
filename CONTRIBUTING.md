# Contributing

Thank you for your interest in Simple White Noise.

## Developer Certificate of Origin

Every commit in a pull request must be signed off under the Developer Certificate of Origin 1.1
(https://developercertificate.org/). Add a `Signed-off-by` line with your real name and email address by committing
with `git commit -s`.

By signing off, you certify that you wrote the change or otherwise have the right to submit it under the project's
license, GPL-2.0-or-later.

## Before you open a pull request

- Build the plugin and run `ctest` as described in the README.
- Format C and C++ sources with clang-format 19, and CMake files with gersemi 0.21.0, the versions the CI uses.
- Do not include private information such as local paths, stream keys, or exported OBS profiles.
