# Sweep Pro display module

This ZMK module provides the e-ink status screen used by NXTKB Sweep Pro. It
contains the display shield, standard keyboard status widgets, optional Cirque
trackpad status, and optional Codex Agent status.

## Optional Codex widget

The Codex display widget is independent from the Codex transport and key
behaviors. It is enabled by default when `CONFIG_NXTKB_CODEX_MICRO=y`, but a
keyboard can keep all Codex controls and hide only the display widget:

```conf
CONFIG_CUSTOM_WIDGET_CODEX_STATUS=n
```

`widgets/aux_status_layout.c` is a layout controller shared by the optional
Codex and trackpad rows; it is not itself a visible widget. Each visible row is
created only when its own Kconfig option is enabled, and the controller is
omitted entirely when both rows are disabled.

## Integration CI

The checked-in `ci/` directory is the production integration fixture for this
module. It pins known Sweep Pro, ZMK, and Codex revisions and builds:

- the ordinary display;
- the display with Codex Agent status;
- Codex firmware with the Agent status widget explicitly disabled.

This verifies both the standalone display path and the optional-widget boundary
before changes are merged.
