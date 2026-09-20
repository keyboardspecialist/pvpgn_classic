# Diablo II Guild Insignias

These PNGs are decoded from the original
`data\global\ui\Emblems\iconXXa.dc6` and `iconXXc.dc6` assets in
`d2data.mpq`.

Source `d2data.mpq` SHA-256:
`BB3835A14AA3935B2204D32DCA9E2409F51507A35679F889A97526240CE6F055`.

- `icon00a.png` through `icon31a.png` are the `48x64` large/banner forms.
- `icon00c.png` through `icon31c.png` are the `16x22` compact selector/lobby
  forms.
- Original palette index `1` is rendered gold (`#E6AE36`).
- Original palette index `2` is rendered blue (`#367DC2`).

The colors are visualization placeholders. Diablo II applies the two guild
color bytes at runtime. The protocol's emblem number is one-based, so emblem
`0x01` selects `icon00`, and emblem `0x20` selects `icon31`.

The PNG exporter preserves DC6 transparency, but these 64 frames contain no
transparent pixels: every pixel uses palette index `1` or `2`. Both channels
therefore remain opaque rather than treating either runtime-selected color as a
background to discard.
