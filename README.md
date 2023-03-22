# RealScript

AST-walking interpreter

Dependency graph:
```
[execute/eval] ───► [execute/env] ──────────┐
    │    │                │                 │
    │    │                ▼                 ▼
    │    └───────► [execute/term] ───► [rational/rational] ───► [rational/alnat]
    │                         │
    │          [parse/parse]  │
    │                    │    │
    │                    ▼    ▼
    └───────────────► [parse/expr] ───► [parse/dict]
```