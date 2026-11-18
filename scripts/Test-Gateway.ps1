$ErrorActionPreference = "Stop"
if (-not $env:ANTHROPIC_BASE_URL) { throw "Run Set-PremiumPortal-Session.ps1 first." }
if (-not $env:ANTHROPIC_AUTH_TOKEN) { throw "Missing ANTHROPIC_AUTH_TOKEN." }
if (-not (Get-Command claude -ErrorAction SilentlyContinue)) { throw "Claude Code CLI not found." }

Write-Host "Base URL: $env:ANTHROPIC_BASE_URL"
Write-Host "Testing the gateway through Claude Code with DeepSeek Flash..."
claude --model "deepseek-v4.1-flash" --print "Reply with exactly: PREMIUM_PORTAL_CONNECTION_OK"
if ($LASTEXITCODE -ne 0) {
  throw "Claude Code request failed. Check gateway protocol, URL path, token, and model ID."
}
