# Distribution source inventory — 24 November 2026

30 worktrees and 32 repository indexes inspected; no branches/worktrees removed.
18 worktrees had local changes and were preserved.

| Source | Status / purpose |
|---|---|
| HTTP8089/full | Canonical; 1945 before consolidation |
| Macintosh Exchange/Clarice | 1943 before consolidation; exact canonical mirror |
| HTTP8080 | Historical physical updater packages in Clarice/work/hardware-update/http |
| HTTP8086 | Historical SE repositories: 1823/1841/1896 |
| HTTP8098 | Historical PB repositories: 1882/1902 |
| HTTP8083/8084 | Snow repository and slow/interruption fixtures, 1801–1823 |
| HTTP8087 | Nano Snow fixtures; repository 1823 |
| Editor-Damage work/editor-snow/http | Diagnostic 1944; not a distribution |
| Vi-Response work/editor-snow/http | Experimental 1941 |
| Other worktree indexes | Historical/experimental 1823–1940; isolated fixtures |

Only8089/full is the official update source. Other listeners are retained because
legacy/test consumers have not been disproven. No remote Mac configuration was
changed; no listener retired speculatively. Build directories in those worktrees
are not publication inputs. The supported full build moves the previous work/build
aside before compiling and sealing the complete inventory.
