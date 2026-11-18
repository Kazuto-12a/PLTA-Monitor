param([Parameter(Mandatory=$true)][string]$Path)
$ErrorActionPreference = "Stop"
git worktree list
Write-Host "Review the worktree and ensure changes are committed or safely preserved."
$confirm = Read-Host "Type REMOVE to remove this worktree"
if ($confirm -ne "REMOVE") { Write-Host "Cancelled."; exit 0 }
git worktree remove $Path
if ($LASTEXITCODE -ne 0) { throw "Removal failed. Resolve uncommitted changes first." }
