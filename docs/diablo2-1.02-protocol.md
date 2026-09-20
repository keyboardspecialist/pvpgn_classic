# Diablo II 1.02 Protocol Notes

This document records the exact identity and validated wire-protocol behavior
for Diablo II Classic 1.02. The complete BNCS, MCP, game, and save lifecycle
has been validated with a live 1.02 client.

## Client identity

The analyzed retail executable reports version `1.0.2.0`. The expected BNCS
version ID is `0x02`, following the observed `0x00`, `0x01`, and `0x03`
sequence. Live authentication confirms that identity.

```text
Version tag:       D2DV_102
Packed version:    0x01000200
CheckRevision MPQ: IX86ver1.mpq
CheckRevision:     0x0BA6BBCD
Game.exe SHA-256:  EE1DD916A917A43C15361FE2B887AFF21C4DAF9FD8C8AF39E05318B0AB4099D7
```

The configured equation is:

```text
A=3845581634 B=880823580 C=1363937103 4 A=A-S B=B-C C=C-A A=A-B
```

The checksum was reproduced from these exact inputs, in order:

```text
Game.exe      EE1DD916A917A43C15361FE2B887AFF21C4DAF9FD8C8AF39E05318B0AB4099D7
Bnclient.dll  12286F43A32E7A9CDB57E7E0E9AA57EA5494CABB76B24734BE0DF28416F9B84D
D2Client.dll  F4CC816C9C863F77A529B19366623A2F6DD9981598C7C3E61366509E44DCD6B9
```

The configured metadata string is
`Game.exe 05/31/21 11:23:26 346243`. Its timestamp belongs to the supplied
archive rather than a verified retail installation. PvPGN does not use
`fileMetadata` for version acceptance.

## BNCS capability

`conn_is_legacy_d2_client()` classifies `D2DV` version ID `0x02` as an early
client only after CheckRevision resolves the exact `D2DV_102` identity. This
selects the same recipient-specific capabilities already used by 1.00 and
1.01:

- SID `0x35` realm handoff.
- SID `0x37` closed-character selector.
- 43-byte early lobby portrait records.
- The otherwise unused three-byte guild field as display tag `102`.

The classification is an explicit version list, not an open numerical range.

## MCP and lobby formats

The analyzed 1.01 and 1.02 `D2MCPClient.dll` request code is byte-identical
apart from PE timestamp/checksum fields. Static analysis indicates the same
51-byte early MCP login, early character-list stream, four-byte ladder request,
and early lobby portrait format documented in
`diablo2-1.00-1.01-protocol.md`.

D2CS does not receive the BNCS version ID. It classifies the complete MCP login
packet shape and stores the existing early-protocol capability, so no new MCP
serializer or D2CS structure is required for 1.02.

## D2Net game join

The analyzed D2Client join path constructs the same 32-byte client frame and
28-byte D2Game payload used by 1.00 and 1.01:

```text
Offset  Size  Field
0x00    1     0xD2
0x01    1     channel zero
0x02    2     total size, 0x0020
0x04    1     join opcode, 0x01
0x05    4     game token
0x09    2     game ID
0x0B    1     character class
0x0C    4     client/protocol version
0x10   16     character-name slot
```

The 1.02 D2Net parser begins `83 FA 04 56 73 09`, matching the measured 1.01
early ingress implementation. A live client completed game creation, token
validation, character retrieval, and entry through the alternate listener.

## Character saves

D2DBS selects legacy handling from the save-file version rather than the BNCS
version ID. Saves below `0x5C` use the established early offsets and skip the
later checksum requirement.

PvPGN's shared 130-byte `newbie.save` template uses revision `0x59`. D2Game
1.02 rejects a newly created character with that revision before its enter-game
callback. For connections using the detected early MCP layout, D2CS now writes
revision `0x47` into the new save while preserving the shared template for later
clients. The deployed implementation created the 130-byte revision-`0x47`
Amazon `zona`; D2Game accepted it, entered at level 1, and wrote an 873-byte
revision-`0x47` save. A second game loaded that complete save and wrote it
again successfully. D2DBS persisted the save and character information,
created backups, and unlocked the character after both games.

## Validation status

| Area | 1.02 status |
|---|---|
| Version/checksum calculation | Reproduced statically |
| BNCS version ID `0x02` | Validated |
| SID `0x35`/`0x37` handling | Validated end to end |
| Early MCP login and replies | Validated |
| Lobby portrait and `102` tag | Validated |
| 32-byte D2Net join | Validated through D2GS handoff and entry |
| D2GS engine startup | Validated |
| Character create/enter/save/reload lifecycle | Validated with revision `0x47` |

## Implementation map

- `conf/versioncheck.json.in` defines the exact `D2DV_102` identity.
- `src/bnetd/connection.cpp` includes version ID `0x02` in the explicit early
  client capability predicate.
- `src/bnetd/handle_bnet.cpp` owns the shared SID `0x35` and `0x37` paths.
- `src/bnetd/message.cpp` owns recipient-specific portrait conversion and the
  three-digit patch tag.
- `src/d2cs/handle_d2cs.cpp` classifies the early MCP login and emits early
  character-list and ladder streams, and supplies that capability when creating
  a character.
- `src/d2cs/d2charfile.cpp` writes save revision `0x47` for early clients while
  retaining the configured template revision for later clients.
- `src/d2dbs/dbspacket.cpp` owns the save-version checksum boundary.

The exact engine hashes, resolver ordinals, Fog mitigation, callback ABI, and
D2Game return patches are recorded in
`docs/classic-1.02-static-assessment.md` in the companion D2GS repository.
