# Diablo II 1.00 Compatibility

This source tree contains the PvPGN, D2CS, and D2DBS changes required by the
hash-gated classic 1.00 D2GS adapter maintained in the companion `d2gs109`
repository.

## Character selection

Diablo II 1.00 requests its initial closed-character list from BNCS with
SID `0x37` before it has selected or joined a realm. The original PvPGN path
looked up account metadata using `conn_get_realm(c)`, which is null at this
point, and returned an empty list.

For the early `D2DV` 1.00, 1.01, and exact-identity 1.02 clients (`versionid`
`0`, `1`, and `2`), BNCS now reads the authoritative D2CS charinfo files from
`d2cs_charinfo_dir`. It returns at most eight records containing:

```text
RealmName,CharacterName\0
43-byte classic portrait plus optional three-byte guild tag\0
```

Modern 34-byte portraits are converted to the classic fixed layout. Existing
classic portraits are preserved. Later clients retain the original PvPGN
character-list path.

The deployment configuration is:

```ini
d2cs_charinfo_dir = var\charinfo
```

D2CS also recognizes the shorter early login structure and supports the legacy
MCP `0x10` character summary message. The BNCS SID `0x37` response remains the
message that populates the pre-realm 1.00/1.01/1.02 selector.

## Character storage

The shared charinfo portrait allocation is 64 bytes so it can hold either the
34-byte modern portrait or the classic 43-byte layout and guild tag. The
adjacent historical padding was absorbed into the portrait field, preserving
the complete on-disk `t_d2charinfo_file` ABI at 192 bytes and the summary at
offset 176.

Legacy Diablo II saves use versions below `0x5C` and do not contain the modern
checksum field at offset `0x0C`. D2DBS validates checksums only for version
`0x5C` and newer, matching the format boundary already used by D2CS.

## Windows build

The verified build uses Visual Studio 2022, Win32, and the bundled x86 zlib:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" `
  -S E:\d2gsbuilder\source `
  -B E:\d2gsbuilder\build-v143 `
  -G "Visual Studio 17 2022" -A Win32 `
  -DWITH_BNETD=ON -DWITH_D2CS=ON -DWITH_D2DBS=ON `
  -DWITH_WIN32_GUI=OFF `
  -DZLIB_INCLUDE_DIR=E:\d2gsbuilder\module\include\zlib\1.2.11 `
  -DZLIB_LIBRARY_RELEASE=E:\d2gsbuilder\module\include\zlib\1.2.11\zdll.lib `
  -DZLIB_LIBRARY_DEBUG=E:\d2gsbuilder\module\include\zlib\1.2.11\zdll.lib

& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  E:\d2gsbuilder\build-v143\pvpgn.sln /m `
  /p:Configuration=RelWithDebInfo /p:Platform=Win32
```

## Validated lifecycle

The integration test on September 18, 2026 verified:

- BNCS returned eight account characters with correct classic portraits.
- The selected character completed the BNCS-to-D2CS realm handoff.
- D2CS authenticated the character and created and joined a classic game.
- D2DBS loaded a legacy save without applying a modern checksum check.
- D2GS saved an updated 873-byte character file and 192-byte charinfo file.
- D2DBS accepted both writes, updated the ladder, and unlocked the character.
- The final player left and the game closed normally.

The corresponding D2GS ABI, D2Net, callback, and exact-hash details are in
`docs/classic-1.09-adapter.md` in the companion repository.

The byte-level BNCS, MCP, ladder, portrait, and D2Net formats shared or
compared with 1.01 are recorded in `docs/diablo2-1.00-1.01-protocol.md`.
