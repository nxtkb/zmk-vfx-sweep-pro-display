# Integration fixtures

This directory is part of the repository's production CI setup. It assembles
the display module with the pinned Sweep Pro configuration and Codex module.
The matrix verifies three supported configurations:

- the display without Codex;
- the display with Codex Agent status;
- Codex firmware with the Agent status widget explicitly disabled through
  `CONFIG_CUSTOM_WIDGET_CODEX_STATUS=n`.

These files are build fixtures, not runtime firmware sources. They can be
updated independently when the pinned integration revisions change.
