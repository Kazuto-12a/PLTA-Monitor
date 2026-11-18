param(
  [Parameter(Mandatory=$true)][string]$ModelId,
  [Parameter(Mandatory=$true)][string]$Agent,
  [string]$Worktree = ""
)

$ErrorActionPreference = "Stop"

if (-not $env:ANTHROPIC_BASE_URL) {
  throw "Set ANTHROPIC_BASE_URL in this PowerShell session first."
}
if (-not $env:ANTHROPIC_AUTH_TOKEN -and -not $env:ANTHROPIC_API_KEY) {
  throw "Set ANTHROPIC_AUTH_TOKEN or ANTHROPIC_API_KEY securely first."
}
if ($Worktree -and (Test-Path $Worktree)) {
  Set-Location $Worktree
}
if (-not (Get-Command claude -ErrorAction SilentlyContinue)) {
  throw "Claude Code CLI was not found. Install it and reopen PowerShell."
}

Write-Host "Starting agent '$Agent' using model '$ModelId' in $(Get-Location)"
claude --model $ModelId --agent $Agent
