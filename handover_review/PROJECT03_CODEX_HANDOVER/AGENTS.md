# PROJECT 03 handover rules

Read `00_START_HERE.md` first. The approved architecture is the complete V1.1 in
`04_ARCHITECTURE_BASELINE.md`, with ADR001–ADR010 and the restricted ADR008 contract.
Read `07_IMPLEMENTATION_STATUS.md`, `08_M1A_EXECUTION_PLAN.md`,
`09_TEST_AND_ACCEPTANCE.md`, and `15_CODEX_WORKING_RULES.md` before implementation work.

This folder contains immutable originals and an **unaccepted M1A candidate**.
Do not rewrite its history as “no implementation”, and do not declare M1A accepted.
Only M1A was authorized; M1B–M1F are not authorized. The present task is handover
documentation only, with no business-code changes. Follow subsequent explicit user tasks.
Do not modify ADRs or silently expand scope. Preserve original archives and historical
logs; write new build/test results separately. Never infer CTP behavior from fixtures or LocalSim.

Run `python3 tools/verify_handover.py` for package integrity. This does not execute
business tests or certify architecture/production readiness. Reference model results
(38 checks) and candidate results (85 tests) are distinct evidence layers.
