# Diablo II 1.00 and 1.01 Protocol Notes

This document records the early retail Diablo II wire formats recovered while
adding 1.00 and 1.01 support. It distinguishes formats proven by static client
analysis from behavior validated against a live client. The complete validated
1.00 lifecycle is summarized in `diablo2-1.00-compatibility.md`.

## Client identity

PvPGN's version-check database assigns these base-game builds to distinct
`D2DV` version IDs:

| Build | Version ID | Version tag | CheckRevision hash |
|---|---:|---|---:|
| 1.00 installation | `0x00` | `D2DV_100` | `0xAC5E46CB` |
| 1.01 retail | `0x01` | `D2DV_101` | `0x5AEF7E66` |
| 1.01 VersionChanger NoCD | `0x01` | `D2DV_101` | `0xDA412BA6` |

The analyzed 1.01 `Game.exe` reports version `1.0.0.1`, has PE timestamp
`0x3957D64D`, and has SHA-256:

```text
CC9E64D0CAC0E667B7F361B62080D7F2AE33C1360E73256AA123FFFB4E7A3DA0
```

The protocol-facing 1.01 binaries and patch archive are:

```text
D2MCPClient.dll  2AA6C3B2989F21F9C3DE212B375B21202EB90DDF90A3D9301353B0C7058AB9BE
D2Multi.dll      AE8CFBB255E64C8E9E41C4BF54CD975E716BF98BBBA5AF2CB7F50DFC149BAD0C
D2Client.dll     1F7799A04E15C2C7B2028EF79B0ECF81640D075CB745BB5A03F9261EBBF67A68
D2Net.dll        33CD7570450DD582EA5A7077B63572D82DF44F368828A5E19650B9E66D81B8C5
Patch_D2.mpq     4CEEE60C72401092B533D11CEB37BD11946DA6C7971263A4127335CC57D2F66E
```

The complete CheckRevision entry uses `IX86ver1.mpq`, equation
`A=3845581634 B=880823580 C=1363937103 4 A=A-S B=B-C C=C-A A=A-B`, and
metadata string `Game.exe 06/26/00 22:31:00 346243`.

Live cross-version selector validation also identified the VersionChanger NoCD
variant. It reports `Game.exe 05/31/21 11:23:26 45056` and CheckRevision hash
`0xDA412BA6`; it shares the exact patch identity `D2DV_101`.

PvPGN treats base-game `D2DV` version IDs `0`, `1`, and `2` as early clients.
Version ID `2` is the exact-identity-gated 1.02 profile and is validated with a
live client. Expansion clients are not included in this capability check. The
1.02 evidence is recorded in `diablo2-1.02-protocol.md`.

## Packet framing

BNCS packets use their normal `FF <opcode> <uint16 size>` header. MCP/D2CS
packets use this three-byte header:

```text
Offset  Size  Field
0x00    2     total packet size, little-endian
0x02    1     opcode
```

D2Net client-to-game packets use this outer envelope:

```text
Offset  Size  Field
0x00    1     0xD2
0x01    1     channel
0x02    2     payload size plus four, little-endian
0x04    n     payload
```

## BNCS realm and character selection

The validated 1.00 client uses SID `0x35` for its realm-login exchange rather
than the later SID_REALMJOIN path. Its reply payload, after the BNCS header, is:

```text
Offset  Size  Field
0x00    4     sequence number
0x04    4     reserved/result
0x08    4     BNCS address
0x0C    4     session number
0x10    4     D2CS address
0x14    2     D2CS port, network byte order
0x16    2     reserved
0x18    4     session key
0x1C    4     reserved
0x20    4     reserved
0x24   20     five-word secret hash
0x38    n     NUL-terminated account name
```

Before joining the realm, 1.00 sends SID `0x37` to request its closed-character
selector. PvPGN returns this fixed header followed by character records:

```text
Offset  Size  Field
0x00    4     zero
0x04    4     maximum characters, currently 8
0x08    4     returned character count
0x0C    n     character records
```

Each record is:

```text
RealmName,CharacterName\0
43-byte early portrait
optional three-byte guild/version tag
\0
```

The early portrait is the non-NUL 43-byte portion of the historical
`t_d2char_info` layout. Unknown and unused bytes must remain nonzero because
the client treats NUL as the record delimiter. PvPGN stores up to 64 portrait
bytes while preserving the 192-byte on-disk charinfo ABI.

The 1.01 live test completed both the early realm handoff and character
selector paths. The current implementation therefore sends 1.01 through the
same measured early-client SID `0x35`/`0x37` handling as 1.00.

## MCP login classification

The early MCP login structure has a 51-byte fixed part followed by the
NUL-terminated account name:

```text
Offset  Size  Field
0x00    3     MCP header, opcode 0x01
0x03    4     sequence number
0x07    4     reserved
0x0B    4     BNCS address
0x0F    4     session number
0x13    4     session key
0x17    4     CD-key ID
0x1B    4     reserved
0x1F   20     five-word secret hash
0x33    n     NUL-terminated account name
```

Later clients insert four additional 32-bit fields before the secret hash.
D2CS therefore detects the early layout from the complete packet shape,
normalizes it into the later internal structure, and records the connection as
early protocol. This layout is live-validated for both 1.00 and 1.01.

## MCP request constructors

Static disassembly proves that 1.00 and 1.01 use identical constructors for
these requests:

```text
03 00 10       character-list request
04 00 11 TT    ladder request; TT is the ladder-type byte
03 00 12       message-of-the-day request
```

The constructor addresses in the analyzed clients are:

| Request | 1.00 | 1.01 |
|---|---:|---:|
| Character list `0x10` | `0x100031B0` | `0x100031B0` |
| Ladder `0x11` | `0x100031F0` | `0x100031F0` |
| MOTD `0x12` | `0x10003230` | `0x10003230` |

This proves that an early ladder request is exactly four bytes. It has no
`start_pos` field.

## MCP character-list reply

The early character-list reply uses opcode `0x10`. Its transport header is:

```text
Offset  Size  Field
0x00    3     MCP header
0x03    1     reserved, zero
0x04    2     total assembled-data length
0x06    2     data length in this packet
0x08    2     data length sent by previous packets
0x0A    n     assembled data
```

The assembled data begins with:

```text
Offset  Size  Field
0x00    4     character count
0x04    4     character-name slot width, 16
```

Each character record occupies 28 bytes:

```text
Offset  Size  Field
0x00    8     expiration as Windows FILETIME
0x08    4     summary
0x0C   16     NUL-terminated character-name slot
```

The summary uses bits `0..3` for class, bit `4` for hardcore, bit `5` for
dead, and bits `16..23` for level. PvPGN returns at most eight early records.
The response implementation is live-validated with both 1.00 and 1.01.

## MCP ladder reply

Both early and later ladder replies use opcode `0x11` and this transport
header:

```text
Offset  Size  Field
0x00    3     MCP header
0x03    1     requested ladder type
0x04    2     total assembled-data length
0x06    2     data length in this packet
0x08    2     data length sent by previous packets
0x0A    n     assembled data
```

For an early client, the assembled stream starts with:

```text
Offset  Size  Field
0x00    4     entry count
0x04    4     character-name width, 16
```

Each following ladder entry is 28 bytes:

```text
Offset  Size  Field
0x00    4     experience, low word
0x04    4     experience, high word; normally zero
0x08    2     status flags
0x0A    1     level
0x0B    1     reserved, zero
0x0C   16     character-name slot
```

PvPGN sends at most 14 entries per transport packet and updates the
continuation length by assembled bytes, not packet bytes. The 1.00 client has
live-validated this format across multiple ladder types. The identical 1.01
request constructor strongly supports reuse, but reply parsing still needs a
live 1.01 test.

## Lobby portrait negotiation

Lobby player-info messages are formatted for the receiving client rather than
the sending client:

| Destination | Portrait bytes before trailing tag/NUL |
|---|---:|
| Early `D2DV_100`/`D2DV_101`/`D2DV_102` | 43 |
| Later D2 client | 33 |

When an early destination receives a Diablo II player whose version ID is at
most 99, PvPGN appends a three-digit display tag. Version IDs `0`, `1`, and `9`
therefore render as `100`, `101`, and `109`. Live clients have validated the
`100`, `101`, and `109` tags. Stock 1.09, 1.09b, and 1.09d clients ignore the
early trailing guild field.

## D2Net game join

Static analysis proves that 1.00 and 1.01 send the same 32-byte game-join
frame:

```text
Offset  Size  Field
0x00    1     0xD2
0x01    1     channel/subtype, zero
0x02    2     total size, 0x0020
0x04    1     join opcode, 0x01
0x05    4     game token
0x09    2     game ID
0x0B    1     class: Amazon=0, Sorceress=1, Necromancer=2,
              Paladin=3, Barbarian=4
0x0C    4     client/protocol version
0x10   16     character-name slot
```

The constructors are `0x10014C70` in 1.00 D2Client and `0x100148B0` in 1.01
D2Client. Both pass a 28-byte payload to D2Net ordinal `10005` on channel zero.
That ordinal is implemented at `0x10001B90` in 1.00 and `0x10001BF0` in 1.01.
D2Net ordinal `10000` initializes network state and is not the join sender.

A captured 1.00 frame decodes as follows:

```text
D2 00 20 00 01 E2 5E 13 BE 4C 02 04 CA 00 00 00
6C 69 6C 6A 6F 65 00 00 64 25 42 00 1C E9 50 00

token:     0xBE135EE2
game ID:   0x024C
class:     Barbarian
version:   0x000000CA
character: liljoe
```

The client does not clear the complete name slot. Bytes after the first NUL
may contain stack residue and must not be interpreted as another field. The
version comes from interface method `[vtable+0x58]`; it is not hardcoded by the
join constructor.

## Save-format boundary

D2DBS treats save versions below `0x5C` as legacy files and does not apply the
modern checksum-at-offset-`0x0C` rule. This behavior is validated with 1.00 and
1.01. The 1.01 client wrote an 846-byte version-`0x47` save, then selected,
loaded, and saved it again successfully.

## Validation status

| Area | 1.00 | 1.01 |
|---|---|---|
| Version-check identity | Validated | Validated as `D2DV_101` |
| BNCS SID `0x35` realm handoff | Validated | Validated end to end |
| BNCS SID `0x37` selector | Validated | Validated end to end |
| 51-byte MCP login layout | Validated | Validated as the early layout |
| MCP `0x10`/`0x11`/`0x12` requests | Validated | Proven by disassembly |
| MCP character-list reply | Validated | Validated with two characters |
| MCP ladder reply | Validated | Pending live decode |
| Lobby portrait/guild tag | Validated | `101` tag validated live |
| 32-byte D2Net join | Validated | Validated with two live game handoffs |
| Legacy save/checksum behavior | Validated | Validated through save and reload |

## Implementation map

- `conf/versioncheck.json.in` identifies `D2DV_100`, `D2DV_101`, and
  `D2DV_102`.
- `src/common/bnet_protocol.h` defines the early SID `0x35` and `0x37`
  structures.
- `src/common/d2cs_protocol.h` defines the MCP login, character-list, and
  ladder structures.
- `src/bnetd/handle_bnet.cpp` handles the early realm and pre-realm character
  selector paths.
- `src/bnetd/message.cpp` performs recipient-specific portrait conversion and
  version-tag formatting.
- `src/d2cs/handle_d2cs.cpp` classifies the MCP login layout and assembles the
  early character-list and ladder streams.
- `src/d2dbs/dbspacket.cpp` applies the legacy save-checksum boundary.

The exact 1.01 engine ABI and binary patch profile are recorded in
`docs/classic-1.01-profile.md` in the companion D2GS repository.
