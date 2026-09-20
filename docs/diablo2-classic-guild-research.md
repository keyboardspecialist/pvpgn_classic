# Diablo II Classic Guild Research

This document records static evidence for Diablo II's unfinished guild system.
It separates fields and assets proven to exist from behavior that still needs a
live client test. PvPGN does not currently implement guild creation, membership,
Guild Hall persistence, or insignia selection.

## Character portrait metadata

The 43-byte early character portrait is immediately followed by an optional
three-character guild tag and its NUL terminator. Its trailing fields are:

| Record-data offset | Size | PvPGN field | Meaning |
|---:|---:|---|---|
| `0x27` | 1 | `emblembgc` | Guild insignia background color |
| `0x28` | 1 | `emblemfgc` | Guild insignia foreground color |
| `0x29` | 1 | `emblemnum` | Guild insignia type number |
| `0x2A` | 1 | `unknownb14` | Unknown, non-NUL filler when unused |
| `0x2B` | 0-3 | `guildname` | Guild tag/abbreviation |
| following | 1 | - | NUL record delimiter |

`src/common/bnet_protocol.h` identifies the three emblem bytes explicitly and
maps `emblemnum` to `D2DATA.MPQ/data/global/ui/Emblems/iconXXa.dc6`, where the
asset number is `emblemnum - 1`. The historical defaults in
`src/bnetd/character.cpp` use `0x80, 0xFF, 0xFF`, which suppresses the emblem.

The fields are additive, not alternatives. `character_get_playerinfo()` writes
the three emblem values into the fixed portrait and then appends `guildname`.
There is no union, replacement rule, or mutual-exclusion flag between them.
The version-provenance tag used by the multi-patch lobby occupies only the
trailing tag bytes and does not overwrite the emblem colors or number.

No known structure or client asset uses the term `chevron`. The supported name
is emblem or insignia. Offset `0x2A` remains unknown and must not be labeled as
a separate chevron field without additional evidence.

## Diablo II 1.07 assets

The disc-only 1.07 installation identified by `Game.exe` product version
`1.0.7.0` retains a substantial guild implementation in `d2data.mpq`. Static
extraction found 32 selectable insignias:

```text
Game.exe   C8AA47AED0D3CF54FA545D0A16A6E90035F51CAEC4716F7F9057B71DE8A85880
d2data.mpq BB3835A14AA3935B2204D32DCA9E2409F51507A35679F889A97526240CE6F055
```

```text
data\global\ui\Emblems\icon00a.dc6 ... icon31a.dc6
data\global\ui\Emblems\icon00c.dc6 ... icon31c.dc6
```

Every `a` asset is a `48x64` one-frame image. Every `c` asset is a `16x22`
one-frame image. All 64 images use only nontransparent palette indices `1` and
`2`, matching the two independently supplied color bytes. The paired sizes
strongly indicate that the same selected insignia was intended for both a large
Guild Hall/banner presentation and a compact lobby or panel presentation.
Decoded PNGs for both variants are in
`docs/assets/diablo2-guild-insignias`.

Additional 1.07 assets include:

| Asset | Decoded shape |
|---|---|
| `Emblems\GuildIconFlag.dc6` | `17x24`, 12 frames |
| `BIGMENU\guild2.dc6` through `guild6.dc6` | `256x256`, 4 frames each |
| `BIGMENU\GuildLadderbckg.dc6` | `256x256`, 4 frames |
| `PANEL\guildiconbckg.dc6` | `256x256`, 6 frames |
| `PANEL\guildvault.dc6` | `256x256`, 4 frames |

This is concrete client-side icon evidence rather than only a reverse-engineered
field name.

## Client text evidence

The 1.07 `data\local\LNG\ENG\Guildinput.txt` resource contains separate UI
labels for `Guild Name` and `Guild Tag`, and calls the graphic an insignia:

```text
Guild Name
Guild Tag
Click here to change insignia.
```

Its invitation template also sends the name and tag as separate values:

```text
You have been invited to join %s {%s}!  Type /d2guildjoin to join.
```

Other retained strings cover `CREATE GUILD`, `ENTER GUILD HALL`, invitations,
the Guild Ladder, member roles, donated gold, and the planned Emblem, Personal
Stash, Vault, Message Board, and Trophy Case upgrades. These resources show that
the tag and insignia belonged to the same larger guild model.

## Other protocol remnants

`t_d2game_gameflag::flag3` documents bit `0x01` as `have guild`. D2GS also
exports a `SaveDatabaseGuild` callback, but the current implementation only logs
the call and returns. These remnants do not establish a complete packet or
database protocol.

## Current conclusion

The evidence supports this model:

1. A guild has a name and a separate three-character tag.
2. It also has one of 32 insignia shapes and two independently selected colors.
3. The portrait protocol can carry the insignia and tag at the same time.
4. The current three-character patch tag can coexist with emblem metadata.

## Diablo II 1.00 live validation

A live selector test on September 20, 2026 used the VersionChanger 1.00 NoCD
client and the existing server character `gargalon2/palpal`:

```text
Game.exe SHA-256:       A561DFDBC8FF660E83FD1C74A651E5081731FAED4AD324799756CF04312B35B9
CheckRevision hash:     0x9DEF2E4C
portrait 0x27..0x29:   01 02 0A
trailing tag:           103
```

Emblem `0x0A` selects `icon09`, a diagonally divided shield. The 1.00 selector
rendered its compact, two-color insignia beside the character figure while it
simultaneously rendered `PALPAL {103}` on the character-name line. The server
log confirmed that the same connection matched `D2DV_100` with version ID `0`
and received the seven-character legacy SID `0x37` list. The exact NoCD
identity is recorded in `conf/versioncheck.json.in`; the test did not rely on
the unknown-version fallback after that entry was deployed.

This validates simultaneous insignia and tag rendering in Diablo II 1.00. It
also confirms that the multi-patch version tag does not consume or disable the
guild-insignia fields.

## Diablo II 1.09 live validation

A second selector test used the same unchanged `palpal` charinfo with the
VersionChanger 1.09 NoCD client:

```text
Game.exe SHA-256:       DDD3DC2557A5CFCBB58D7AEA958584030CB4716C8D8A8D2F8CFEA4A0ADCF5429
CheckRevision hash:     0x19BDFF70
stored emblem bytes:    01 02 0A
stored trailing tag:    103
```

The 1.09 selector rendered the same compact insignia but displayed only
`PALPAL`, without `{103}`. After selecting the character, the insignia also
remained visible in the Battle.net lobby. This matches the modern 33-byte
portrait conversion in `d2char_portrait_render()`: legacy offsets
`0x27..0x29` are preserved at modern offsets `0x1D..0x1F`, while the trailing
tag at legacy offset `0x2B` is not part of the modern portrait.

The 1.09 `Patch_D2.mpq` does not replace the guild resources. The original
32 large and 32 compact insignias remain available from the base `d2data.mpq`.
Therefore 1.09 retained insignia shape and two-color rendering compatibility
but removed the separate three-character guild-tag display path. Rendering
behavior in the other pre-1.10 revisions still requires per-version validation.

The live client exposed a cold-start color quirk. On the initial selector and
lobby entry, the insignia used an entirely different color pair than expected.
After attempting to enter a game and returning to the lobby, the same insignia
rendered with the correct colors. The stored charinfo remained byte-for-byte
unchanged at `01 02 0A`, and D2DBS did not save the character during that
transition. The correction is therefore client-side, likely initialization of
a palette or color-translation table when D2Client is loaded, rather than a
portrait conversion or persistence effect.
