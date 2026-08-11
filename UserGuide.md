# User Guide

The guide is no longer a single file. It lives under [`docs/`](docs/index.md)
and is published as a site built with mkdocs-material.

| Topic | Page |
|---|---|
| Overview and the two compile-time claims | [docs/index.md](docs/index.md) |
| The meta-struct and `operator[]` | [docs/struct.md](docs/struct.md) |
| Fields, the four axes, the descriptor table | [docs/schema/index.md](docs/schema/index.md) |
| The size axis | [docs/schema/size-axis.md](docs/schema/size-axis.md) |
| `with_fields`, `compute` and its aliases | [docs/schema/computed-values.md](docs/schema/computed-values.md) |
| `maybe` and `variance` | [docs/schema/optional-and-variant.md](docs/schema/optional-and-variant.md) |
| The constraint DSL | [docs/constraints.md](docs/constraints.md) |
| Reading | [docs/reading.md](docs/reading.md) |
| Writing | [docs/writing.md](docs/writing.md) |
| Custom streams | [docs/streams.md](docs/streams.md) |
| Compile time | [docs/compile-time.md](docs/compile-time.md) |
| Errors | [docs/errors.md](docs/errors.md) |
| Exact declarations and known limitations | [docs/reference.md](docs/reference.md) |

To read it as a site:

```
pip install -r requirements-docs.txt
mkdocs serve
```
