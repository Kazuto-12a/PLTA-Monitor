param(
  [Parameter(Mandatory=$true)][string]$Task,
  [Parameter(Mandatory=$true)][string]$Branch
)
$ErrorActionPreference = "Stop"
git rev-parse --show-toplevel | Out-Null
if ($LASTEXITCODE -ne 0) { throw "Run this inside a Git repository." }

git status --short
if ($LASTEXITCODE -ne 0) { throw "Could not inspect Git status." }

$path = Join-Path (Split-Path (git rev-parse --show-toplevel) -Parent) ("worktrees\" + $Task)
New-Item -ItemType Directory -Force -Path (Split-Path $path -Parent) | Out-Null
git worktree add -b $Branch $path HEAD
if ($LASTEXITCODE -ne 0) { throw "Worktree creation failed." }
Write-Host "Created worktree: $path"
Write-Host "Start an agent with: .\scripts\Start-Agent.ps1 -ModelId <gateway-model-id> -Agent <agent-name> -Worktree '$path'"
