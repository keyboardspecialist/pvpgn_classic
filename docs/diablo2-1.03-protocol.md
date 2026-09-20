# Diablo II 1.03 Protocol Notes

This document records the exact client identity and validated protocol behavior
for Diablo II Classic 1.03. Authentication, character enumeration, character
creation, game entry, save, reload, and a second save have been exercised with
a live client.

## Client identity

The live VersionChanger executable authenticated as:

```text
Version tag:       D2DV_103
Version ID:        0x03
Packed version:    0x01000300
CheckRevision:     0x5548B82D
File metadata:     Game.exe 05/31/21 11:23:26 45056
Game.exe SHA-256:  7F5F17F15DD99BACCB90D5E51EF9DD4745A2682A0A16A509007DF0167FCB2BB4
```

The supplied archive also contains a 346243-byte retail executable with
SHA-256
`924FFDBDDDCC17599B1662BAD5D89009EC0D24DD0CEB5347C0BC727C0C56D1F6`.
Its CheckRevision result is `0x236BEDDD`. The version configuration retains the
previously known `0x0120ED90` variant as well. PvPGN stores all three as entries
under version ID `0x03` and selects the exact entry by packed version and
checksum.

## BNCS capability

`conn_is_legacy_d2_client()` recognizes version ID `0x03` only after version
checking resolves the exact `D2DV_103` tag. It selects the shared early-client
capabilities:

- SID `0x35` realm handoff.
- SID `0x37` closed-character enumeration.
- Early lobby portrait conversion.
- The 51-byte early MCP login layout.

This is an explicit identity check, not an open version range.

## Character selector

The stock 1.03 `D2Launch.dll` SID `0x37` parser reads portrait bytes `43..45`
as the otherwise unused three-byte guild tag. If present, the selector renders
the character as `CharacterName {tag}`.

PvPGN uses this field for character creation provenance. On successful BNCS to
D2CS account authentication, the internal account-login reply carries a packed
three-byte code derived from the exact version tag. `D2DV_103` becomes `103`;
suffix patches use compact values such as `04B`, `04C`, `09B`, and `09D`.
D2CS stores the code and a magic value in two reserved charinfo DWORDs when the
character is created. D2DBS preserves those fields when D2GS later rebuilds
charinfo. SID `0x37` displays the stored code independently of the client that
requests the list. Characters without trustworthy provenance display `???`.

Live validation created `palpal` with the exact 1.03 client and confirmed that
its charinfo was automatically stamped `103`. The exact 1.01 client then
displayed `palpal {103}` while displaying a trusted 1.01 backfill as `{101}`,
confirming that the selector does not substitute the requesting client's
version. A 1.03 game then expanded `palpal` from the 130-byte newbie save to an
873-byte revision-`0x47` save. D2DBS wrote the reconstructed charinfo, retained
the `103` provenance fields, and unlocked the character successfully.

## MCP behavior

Live 1.03 traffic uses the early 51-byte MCP login and the early character-list
stream already implemented for 1.00 through 1.02. Static analysis also shows
the 1.03 queue messages:

```text
0x13  Cancel game request
0x14  Game queue position
```

Both queue messages were already implemented by D2CS. No new public MCP packet
layout was required.

## D2Net and saves

The client completed the established early D2Net handoff through game creation,
token validation, character retrieval, and entry. As with 1.02, D2Game 1.03
requires a revision-`0x47` newly created save rather than the shared modern
revision-`0x59` template. D2CS writes `0x47` for all connections using the early
MCP layout.

The live lifecycle created the Necromancer `bonneezz` as a 130-byte revision-
`0x47` save. D2Game expanded it to 846 bytes, D2DBS persisted it, and a second
game loaded and saved the 846-byte file again. Ladder update, charinfo write,
backup rotation, unlock, and game close all completed.

## Implementation map

- `conf/versioncheck.json.in` defines the exact 1.03 identities.
- `src/bnetd/versioncheck.cpp` permits multiple checksum variants for one
  version tuple.
- `src/bnetd/connection.cpp` explicitly enables the early capability for
  `D2DV_103`.
- `src/bnetd/handle_bnet.cpp` serializes SID `0x35`/`0x37` and displays stored
  creation provenance.
- `src/bnetd/handle_d2cs.cpp` derives and forwards the authenticated patch tag.
- `src/common/d2cs_bnetd_protocol.h` carries the packed tag to D2CS.
- `src/d2cs/d2charfile.cpp` stores provenance and creates revision-`0x47` saves.
- `src/d2dbs/dbspacket.cpp` preserves provenance across charinfo rewrites.

Exact D2GS hashes, patch sites, callback ABI, and runtime gating are documented
in `docs/classic-1.03-profile.md` in the companion D2GS repository.
