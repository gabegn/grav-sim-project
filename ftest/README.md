# Functional test data

Each folder is one test case used by `ftest.sh`. The expected outputs were
worked out by hand, not produced by the program.

| Folder | What it checks | Arguments |
|---|---|---|
| `rebound` | One object crossing the right and bottom walls in the same iteration | `3 100 50 1` |
| `invalid_input` | Junk lines, invalid objects, incomplete last line; skipped lines still use up an id | `2 100 50 0.5` |
| `initial_collisions` | Collisions before the simulation: heavier absorbs, chained merges, tie goes to the smaller id | `1 100 50 1` |
| `collision_in_simulation` | Two objects meet during the simulation and merge | `2 100 50 1` |
| `gravity` | Gravitational acceleration between two heavy objects | `1 100 50 1` |
| `no_valid_objects` | No valid object in the input | `5 100 50 1` |
| `aos_vs_soa` | 300 random objects: AOS and SOA must write identical files | `50 300 300 0.1` |

Objects in the small cases have tiny masses and are far apart, so gravity is too weak
to change any value at three decimals (except in `gravity`). It still pulls a velocity
of exactly 0 slightly below zero, which is printed as `-0.000`: the sign shows the
direction of the pull, so it is part of the expected output.

Test inputs never place an object exactly on a wall: with two objects, the tiny pull
of gravity decides whether the rebound happens in that iteration or the next.
`rebound` uses a single object, so there is no gravity and every value is exact.
