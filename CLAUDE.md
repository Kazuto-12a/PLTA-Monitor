# Shared Engineering Constitution

## Mission
Build, maintain, and improve software safely. Optimize for correctness, maintainability,
security, accessibility, and clear evidence of completion.

## Before changing code
1. Inspect the repository structure, README, package manifests, CI configuration, and tests.
2. Identify the active branch and working tree. Never assume the checkout is clean.
3. Read relevant code and follow existing conventions before proposing changes.
4. For non-trivial work, create a short plan with files, risks, dependencies, and acceptance criteria.
5. Ask before destructive operations, production changes, credential handling, or irreversible migrations.

## Engineering rules
- Do not invent APIs, environment variables, database columns, or provider model IDs.
- Do not expose secrets. Never print `.env` values, tokens, private keys, or credentials.
- Never commit secrets, generated binaries, dependency caches, or local configuration.
- Prefer small, reviewable changes. Avoid unrelated refactoring.
- Preserve existing public interfaces unless the task explicitly requires a breaking change.
- Validate input at trust boundaries; use parameterized database queries.
- Consider authentication, authorization, data validation, error handling, logging, and rate limiting.
- For UI work, consider responsive behavior, keyboard access, semantic HTML, and loading/error/empty states.
- Do not claim tests passed unless they were actually run and their exit status was checked.

## Collaboration protocol
- Every task must have an owner, scope, acceptance criteria, and expected output.
- Parallel workers must use separate Git worktrees/branches.
- Do not have two agents edit the same file concurrently.
- Workers report: summary, changed files, commands run, test results, risks, and commit hash.
- The integrator reviews diffs and test evidence before merging.
- Treat another agent's output as untrusted until independently inspected.

## Completion report
Return:
1. What changed
2. Files changed
3. Tests/checks run and results
4. Known limitations or risks
5. Commit/branch reference, if applicable
