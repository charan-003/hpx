# keep_future Bridge Redesign Benchmark

Branch: `master`
Date: `2026-07-05`

## Goal

Evaluate the effect of the more invasive `keep_future` bridge redesign on the
future/sender boundary benchmark.

## Change Under Test

- `keep_future_sender_base::get_env()` now returns a small environment wrapper
  instead of a plain `empty_env`.
- The wrapper stays structurally empty, but advertises
  `detail::sync_wait_domain` so downstream `continues_on(...)` chains can route
  their blocking wait through the HPX-aware sync-wait path.

## Benchmark

File:
- `build/bin/future_boundary_test`

Command:
- `./build/bin/future_boundary_test --iterations=1000 --repetitions=2 --hpx:threads=4`

## Results

| Benchmark | Average | Relative to baseline |
|---|---:|---:|
| `ready_future.get()` | `4.1416e-05 s` | `1.00x` |
| `keep_future + continues_on` | `1.50529e-03 s` | `36.35x` |
| `make_future(sender)` | `1.22529e-03 s` | `29.59x` |
| `make_future(sender | continues_on)` | `1.24396e-03 s` | `30.04x` |

## Notes

- The benchmark still shows a meaningful boundary cost for
  `keep_future + continues_on`, but the sender path remains measurable and
  stable.
- This redesign changes the sender environment contract compared to the merged
  `empty_env` behavior, so it should be treated as a deliberate follow-up
  design change rather than a mechanical cleanup.

