# Motate

Motate: A high-performance multi-architecture bare-metal framework that is easy to learn and easy to use.

# Before submitting changes

We have standardized on `clang-format` for pre-commit hook to ensure formatting.

Since Motate is often used as a sumbodule, use this incantation in order to install the hooks:

1. Install `clang-format` which should also install the `git-clang-format` utility.
  - On OS X, it's in homebrew: `brew install clang-format`
  - On Linux it should be in the appropriate repo
  - On Windows -- *Please contribute these instructions*
1. Now install the hook:
  - On OSX or linx with a BASH-like shell:
  ```bash
  ln -s ${PWD}/git-hooks/pre-commit `git rev-parse --git-dir`/hooks/pre-commit
  chmod +ux `git rev-parse --git-dir`/hooks/pre-commit
  ```

# Status

Motate is shaping up quickly, but is still in heavy development. It is currently being developed and tested inside the [Synthetos G2 project](https://github.com/synthetos/g2/).

**The interface for this code is still in flux.** Before the Motate system is completed I'll freeze the interface.

Please share any suggestions for changes. Pull requests welcome as well.

# Usage

Please see the [Wiki](https://github.com/synthetos/Motate/wiki).

-Rob
