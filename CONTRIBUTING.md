# Contributing

Thanks for your interest in improving the chord length estimator.

## Getting set up

Follow the [Building](README.md#building) instructions in the README to
configure and build the project locally before making changes.

## Making changes

1. Fork the repo and create a branch off `master`.
2. Make your changes. Keep formatting consistent with the surrounding
   code (existing style: 4-space indent, braces on the same line).
3. Confirm the project still builds:
   ```sh
   cmake --build build -j$(nproc)
   ```
4. If your change affects the UI, run the app and verify it behaves
   as expected before opening a PR.
5. Open a pull request against `master` with a clear description of
   what changed and why. GitHub Actions will build your branch
   automatically — see the PR build badge in the README.

## Reporting issues

Open a GitHub issue with steps to reproduce, what you expected, and
what happened instead. For build failures, include your OS, compiler
version, and the full error output.
