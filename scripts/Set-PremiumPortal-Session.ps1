# Run this in PowerShell before launching Claude Code.
# The token is held only in this PowerShell process, not written to disk.
$ErrorActionPreference = "Stop"
$env:ANTHROPIC_BASE_URL = "https://coding-portal.premiumportal.id/api"
$secure = Read-Host "Enter your Premium Portal API token" -AsSecureString
$env:ANTHROPIC_AUTH_TOKEN = [System.Net.NetworkCredential]::new("", $secure).Password

Write-Host "Premium Portal base URL configured for this PowerShell session."
Write-Host "Next: run .\scripts\Test-Gateway.ps1"
