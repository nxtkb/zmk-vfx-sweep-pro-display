# Sweep Pro display module

This ZMK module provides the e-ink status screen used by NXTKB Sweep Pro. It
contains the display shield, standard keyboard status widgets, optional Cirque
trackpad status, and optional Codex Agent status.

## Split USB power reporting

The battery widget can show a USB power indicator for each half. Reporting the
peripheral's USB power state requires a small official-ZMK patch maintained in
this module, independently of Codex. The patch is tested against official ZMK
`4de313367288aad5aeaa1243890e40dc813f7008`.

When this module and ZMK are west projects, apply the checksum-verified patch
after `west update` and before building:

```sh
west patch -sm zmk-vfx-sweep-pro-display apply
```

For a sibling module outside the west workspace, run `git apply --check` and
`git apply` in the official ZMK checkout with
`zephyr/patches/zmk/zmk-split-peripheral-usb-power.patch` from this module.
Apply the patch once to a clean checkout; `west update` may discard it, so
apply it again after updating ZMK. A ZMK fork is not required.

Enable this option in the shared keyboard configuration for **both halves**:

```conf
CONFIG_ZMK_SPLIT_PERIPHERAL_USB_POWER=y
```

Flash matching central and peripheral firmware. The indicator represents USB
power presence, not the battery charger's charging or full status. Without the
option, the widget retains normal battery and central USB power reporting.

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
- the plain and trackpad peripherals with USB power reporting enabled.

This verifies both the standalone display path and the optional-widget boundary
before changes are merged. The reusable workflow applies the Codex patch and
this module's power patch; `.` in `patch_modules` selects the checked-out display
module itself, including changes under review.
