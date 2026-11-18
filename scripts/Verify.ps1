# Run from repository root. Adjust commands to the project's actual package manager.
$ErrorActionPreference = "Continue"

Write-Host "`n=== Git diff check ==="
git diff --check

Write-Host "`n=== Detect package manager ==="
if (Test-Path "package.json") {
  if (Test-Path "pnpm-lock.yaml") { pnpm test; pnpm lint; pnpm build }
  elseif (Test-Path "yarn.lock") { yarn test; yarn lint; yarn build }
  elseif (Test-Path "package-lock.json") { npm test; npm run lint; npm run build }
  else { npm test; npm run lint; npm run build }
}
elseif (Test-Path "pyproject.toml") {
  python -m pytest
}
elseif (Test-Path "go.mod") {
  go test ./...
}
elseif (Test-Path "Cargo.toml") {
  cargo test
}
else {
  Write-Warning "No recognized project manifest. Run the repository's documented checks manually."
}
