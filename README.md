# Claude Code 12-Model Engineering Starter — Premium Portal edition

## Confirmed from your supplied model list
Gateway base URL: `https://coding-portal.premiumportal.id/api`

Exact model IDs:
- `claude-opus-4-6`
- `claude-haiku-5-5`
- `deepseek-v4-pro`
- `deepseek-v4.1-flash`
- `gemini-3.8-flash`
- `glm-5.3`
- `gpt-6-luna`
- `gpt-6.1-sol`
- `kimi-k3`
- `minimax-m3.1-flash`
- `muse-spark-1.3-contributor`
- `qwen3.8-flash`

## Important protocol check
The model-list JSON you supplied proves that the gateway lists these model IDs.
It does NOT by itself prove that `/api` implements the Anthropic Messages API required
by Claude Code. Claude Code sends Anthropic-format requests (normally to a Messages
endpoint under the configured base URL). Confirm with Premium Portal that this base URL
supports Anthropic Messages compatibility, including tool use and streaming.

If Premium Portal only exposes an OpenAI-compatible Chat Completions endpoint, Claude Code
cannot use it directly through `ANTHROPIC_BASE_URL`; use a supported Anthropic-compatible
endpoint/adapter instead. Do not guess or append `/v1` until Premium Portal confirms the route.

## 1. Set the endpoint and token
Open PowerShell in this extracted folder:
```powershell
.\scripts\Set-PremiumPortal-Session.ps1
```
Enter your token at the secure prompt. It is not saved to disk.
Do not paste tokens into chat, source code, CLAUDE.md, or Git.

## 2. Test the connection
```powershell
.\scripts\Test-Gateway.ps1
```
The test sends one small request using `deepseek-v4.1-flash`.
If it fails, capture the HTTP status and sanitized error message. Never share the token.

## 3. Launch each specialist
Each agent file has its exact model ID in YAML frontmatter. Start the specialist as:
```powershell
claude --model "claude-opus-4-6" --agent architect
claude --model "claude-haiku-5-5" --agent rapid-builder
claude --model "deepseek-v4-pro" --agent reasoning-engineer
claude --model "deepseek-v4.1-flash" --agent fullstack-implementer
claude --model "gemini-3.8-flash" --agent autonomous-engineer
claude --model "glm-5.3" --agent backend-engineer
claude --model "gpt-6-luna" --agent support-engineer
claude --model "gpt-6.1-sol" --agent senior-fullstack
claude --model "kimi-k3" --agent repository-intelligence
claude --model "minimax-m3.1-flash" --agent rapid-implementation
claude --model "muse-spark-1.3-contributor" --agent coordinator
claude --model "qwen3.8-flash" --agent utility-coder
```
The `--model` flag explicitly selects the main session model; the agent file also declares
the intended model. Keep these values aligned.

## 4. Model and tool compatibility test
For every model, verify:
1. Basic text response
2. Streaming response
3. Tool call (e.g. read a file, then report its contents)
4. Multi-step tool use
5. Long prompt/context handling
6. Error and retry behavior

A model may answer chat prompts but still fail tool calling. Do not assign it autonomous
file-editing work until the tool-call test passes.

## 5. Roles
See `model-map.json` for exact ID-to-agent assignments. See `.claude/agents/` for all
12 complete role prompts.

## 6. Safety and Git
- Use a separate Git worktree and branch for every concurrent editing agent.
- Never share one working directory between simultaneous writers.
- Keep `.env`, tokens, private keys, and credentials out of Git.
- Review diffs and run tests before merging.
- Human approval is required for destructive operations, production changes, and migrations.
