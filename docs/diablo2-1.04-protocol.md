# Diablo II 1.04b/1.04c Protocol Notes

This document records the configured retail identities and current static
compatibility implementation for Diablo II Classic 1.04b and 1.04c. Live
authentication and full character lifecycle validation remain pending.

## Client Identity

```text
Patch   Version tag  Version ID  Packed version  CheckRevision  Game.exe SHA-256
1.04b   D2DV_104B    0x04        0x01000401      0x3882960E     50FF5F0370A8E8632DDED10B68F81743330B120A7F2BBF977974A17E4047A97F
1.04b   D2DV_104B    0x04        0x01000401      0xA3C40DCD     98157B1EAAFC10433D3CF7B47E5CAC5B8755709C18A8A8F89FDBC3C57A31216C (VersionChanger NoCD)
1.04c   D2DV_104C    0x04        0x01000402      0xC6D40AAF     A00B99DD9FA3E98B38EF0D44988AFB4E5781309E2CFCD27E81E438D4F61A2072
```

The analyzed executables have these retail identities:

| Patch | File version | Size | PE timestamp |
|---|---|---:|---:|
| 1.04b | `1.0.4.1` | 403587 | `0x3A439611` |
| 1.04c | `1.0.4.2` | 387107 | `0x3A443C3C` |

Both entries use the `IX86ver1.mpq` seed and configured equation already
present in `conf/versioncheck.json.in`. The suffix-bearing tags remain distinct
even though both clients report version ID `0x04`.

The additional 1.04b identity covers VersionChanger's NoCD loader observed in
the live BNCS request. Exact matching remains required; unknown version-ID
`0x04` clients are not promoted to the 1.04 capability path.

## BNCS Capability

`conn_is_legacy_d2_client()` admits version ID `0x04` only after version
checking resolves the exact tag to `D2DV_104B` or `D2DV_104C`. It does not
admit every client with ID `0x04`, and it does not extend the capability through
an open numerical range.

The predicate selects the shared early-client behavior:

- SID `0x35` realm handoff.
- SID `0x37` closed-character enumeration.
- Early lobby portrait conversion.
- The 51-byte early MCP login layout.
- Revision-`0x47` new character saves selected by that MCP layout.

## MCP Protocol Boundary

The 1.04 MCP login request retains the 1.00 through 1.03 layout. Its fixed
portion is 51 bytes including the three-byte MCP header, followed by the
NUL-terminated account name:

```text
Offset  Size  Field
0x00    3     MCP header, opcode 0x01
0x03    4     sequence
0x07    4     unknown
0x0B    4     BNCS address
0x0F    4     session number
0x13    4     session key
0x17    4     CD-key ID
0x1B    4     unknown
0x1F    20    secret hash
0x33    ...   NUL-terminated account name
```

This is proven by the 1.04b request constructor at `D2MCPClient.dll`
`0x6FAE1DF0`. The constructor emits opcode `0x01`, seven DWORDs, the five-DWORD
hash, and the account string. Consequently, the login packet shape alone cannot
distinguish 1.04 from 1.00 through 1.03; D2CS uses the exact patch tag returned
by BNCS after authentication for the later protocol branch.

The character-list exchange changed in 1.04. The 1.04b and 1.04c
`D2MCPClient.dll` files are byte-identical with SHA-256
`07A0FC70165CD5C36966614B8E3A786021A257074034A01BEAF888FA7631B2A4`, so the
same measured MCP protocol applies to both suffix releases:

```text
Client request:
Offset  Size  Field
0x00    3     MCP header, opcode 0x15

Server reply:
Offset  Size  Field
0x00    3     MCP header, opcode 0x15
0x03    4     maximum character count
0x07    4     current character count
0x0B    ...   repeated character-name\0, portrait\0 pairs
```

The request constructor at `0x6FAE29A0` sends exactly one opcode byte before
MCP framing. The receive dispatch entry for `0x15` is `0x6FAE1720`; its parser
at `0x6FAE1D50` caches the payload beginning after the opcode. D2Launch's parser
at `0x6FB044E0` reads the two DWORDs and advances through two NUL-terminated
strings for each character. Its portrait decoder at `0x6FB02E20` consumes the
modern portrait fields through offset `0x1D`, then treats offsets `0x1E` through
`0x20` as guild-emblem controls and copies the three-byte guild tag from offsets
`0x21` through `0x23`. D2CS preserves the character's existing emblem controls
and uses that otherwise unused tag to display stored patch provenance. The
portrait string terminator is at offset `0x24`. The
1.04 receive dispatcher accepts opcodes below `0x17` at `0x6FAE173F`, while
1.03 accepts only opcodes below `0x15` at `0x10001C9F`.

Character login did not move to the new opcodes. The 1.04 constructor at
`0x6FAE1FB0` still sends opcode `0x07` followed by the NUL-terminated character
name. The `0x07` reply handler at `0x6FAE1060` reads the same DWORD result used
by the existing D2CS implementation.

Opcode `0x16` is a separate D2Multi ladder/profile request, not character
login. Its 1.04 constructor at `0x6FAE29D0` sends one DWORD and a NUL-terminated
character name, with the result delivered through the `0x11` ladder path. The
exact meaning of its DWORD remains pending and is not part of the character
selector fix.

Previously D2CS classified 1.04 as the 1.00 protocol solely because both use
the 51-byte login layout. It then pushed the segmented `0x10` character list and
had no handler for the client's `0x15` request. D2CS now retains the shared
early login and save behavior, suppresses the obsolete automatic `0x10` list
for exact `04B`/`04C` sessions, and emits the 1.04 `0x15` list on request.

Character provenance remains exact-tag based. Newly created characters are
stamped `04B` or `04C` in the reserved charinfo fields and the early selector's
otherwise unused guild-tag display exposes the stored patch independently of
the requesting client. Battle.net lobby entries derive the same three-character
display code from each source connection's resolved version tag, so online
1.04b and 1.04c clients appear as `04B` and `04C` rather than both as `104`.

## Engine Boundary

The 1.04b and 1.04c patch directories contain different `Game.exe` files but
byte-identical engine DLL sets. D2GS therefore uses one exact-hash engine
profile for both BNCS identities. Static analysis found a new callback-table
ABI rather than reusing the 1.01 through 1.03 ABI.

The companion D2GS profile documents the exact DLL hashes, host patch values,
Fog diagnostic mitigation, database-return sites, and callback signatures in
`docs/classic-1.04-profile.md`.

## Validation Status

The version entries, explicit capability predicate, executable identities,
shared D2GS engine identity, and MCP `0x15` character-list path are implemented
or statically verified. The Release `d2cs` and `bnetd` targets build
successfully. Live tests must still cover authentication, character enumeration,
lobby rendering, character creation, game entry, save, reload, unlock, ladder
update, and game close for both suffix releases.

## Implementation Map

- `conf/versioncheck.json.in` defines the exact 1.04b and 1.04c identities.
- `src/bnetd/connection.cpp` explicitly enables both tags as early clients.
- `src/bnetd/message.cpp` converts early portraits and appends the exact patch
  display code to mixed-version lobby entries.
- `src/bnetd/handle_bnet.cpp` serializes the early SID `0x35`/`0x37` paths.
- `src/bnetd/handle_d2cs.cpp` forwards the exact authenticated patch tag.
- `src/common/d2cs_protocol.h` defines the 1.04 MCP `0x15` request and reply.
- `src/d2cs/handle_d2cs.cpp` serializes the 1.04 character list.
- `src/d2cs/handle_bnetd.cpp` avoids sending the older `0x10` list to 1.04.
- `src/d2cs/d2charfile.cpp` stores provenance and creates revision-`0x47` saves
  for the early MCP layout.
- `src/d2dbs/dbspacket.cpp` preserves provenance across charinfo rewrites.
