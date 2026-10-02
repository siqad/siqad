# Fixture provenance and expectations

These are deliberately small reconstructed reference documents, not files exported
by running an old SiQAD binary or a scientific simulator. Geometry is independently
specified: DB sites `(0,0,0)`, `(-1,-1,1)`, `(3,0,1)` correspond to positions
`(0,0)`, `(-3.84,-5.43)`, `(11.52,2.25)` angstrom on Si(100) 2x1.

* `legacy-physical.sqd`: root-level `layer_prop`, no lattice coordinates, and
  `layer_id`/`elec`/`physloc` match `DesignPanel::saveToFile`, `Layer::saveLayer`
  and `DBDot::saveItems` at `91628d4199127655e93be694894c9b6cdcdc62f0`
  (the commit immediately before the May 2018 lattice-coordinate transition).
  The origin is valid data, not an absent coordinate.
* `v0.1-lattice.sqd`: the `<layers>` wrapper, lattice plus physical coordinates,
  and absent role/vector/color fields follow the v0.1.0 serializers at
  `87a12b720daa0b51256c1883c9d1381798aa737d` in those same files.
* `v0.2.1-aggregates.sqd`: nested `Aggregate::saveItems` and per-DB ARGB colors,
  with no layer roles or explicit lattice vectors, follow v0.2.1-hotfix1 at
  `39232c189ff6e35136e5ce2104a002ad1a597bb5`.

Tests assert exact site/physical positions, lattice occupancy, default or explicit
colors, layer roles/names/heights, aggregate hierarchy and a current-format
save/reload. They also check that lattice coordinates take precedence over stale
physical metadata. These cover the represented legacy branches, not every historical
item type or every saved GUI preference.

`simulation-results.xml` is the deterministic subprocess result protocol:
three physical sites and two charge configurations with known multiplicities.
It qualifies process/job/visualizer integration, not simulator physics.
`nested-sidbs.sqd`, `db-locations.xml`, and `charge-configs.xml` are the existing
constructed references for current-format documents and direct result parsers.
