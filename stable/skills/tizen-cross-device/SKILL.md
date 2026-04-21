---
name: tizen-cross-device
description: Manages cross-device operations, multi-assistant framework, user activity recognition, and personalized context from nearby devices. Use when working with cross-device communication, multi-assistant coordination, user activity tracking, or retrieving personal data from connected mobile devices.
argument-hint: "[list-tools|execute|connect|disconnect]"
user-invocable: true
allowed-tools: true
---

# Tizen Cross-Device

Manages cross-device operations, multi-assistant framework, user activity recognition, and personalized context from nearby devices.

**Prerequisite for Crossdevice Context:** `com.samsung.tizen.crossdevice-context` service must be running and a mobile device must be connected.

## Quick reference

**Crossdevice Context**: Retrieve personalized context from mobile → `crossdevice-context-cli`
**Multi Assistant**: Multi-assistant framework → `tizen-multi-assistant-cli`
**User Activity**: User activity recognition → `tizen-ua-cli`

## Crossdevice Context

Retrieve personalized context and data from nearby devices (primarily mobile) via D2D (Device-to-Device) communication.

### Intent-Based Workflow

| User says | Action | Example |
|---|---|---|
| "모바일에서 자주 본 영상 추천해줘" | `execute getTopVideoList` | `crossdevice-context-cli execute --category PERSONAL_DATA --feature LifestyleEntertainment --method getTopVideoList` |
| "내가 좋아하는 음악 뭐야?" | `execute getTopMusicList` | `crossdevice-context-cli execute --category PERSONAL_DATA --feature LifestyleEntertainment --method getTopMusicList` |
| "지금 운동 중이야?" | `execute getExerciseContexts` | `crossdevice-context-cli execute --category CONTEXT --feature BasicDomain --method getExerciseContexts` |
| "집에 있어?" | `execute getHomeState` | `crossdevice-context-cli execute --category CONTEXT --feature SystemState --method getHomeState` |
| "최근 방문한 장소 알려줘" | `execute getVisitedPlaces` | `crossdevice-context-cli execute --category CONTEXT --feature BasicDomain --method getVisitedPlaces` |
| "내 프로필 정보 보여줘" | `execute getMyProfile` | `crossdevice-context-cli execute --category PERSONAL_DATA --feature BasicDomain --method getMyProfile` |
| "어떤 도구들 사용 가능해?" | `list-tools` | `crossdevice-context-cli list-tools` |

### CLI Usage

```bash
crossdevice-context-cli <subcommand> [options]
crossdevice-context-cli --help
crossdevice-context-cli --schema
```

### Subcommands

| Subcommand | Description |
|---|---|
| `list-tools` | List all available tools with category, feature, method |
| `execute` | Execute a specific tool method |

### Execute Options

| Option | Description |
|---|---|
| `--category <CATEGORY>` | Tool category: `CONTEXT` or `PERSONAL_DATA` (required) |
| `--feature <FEATURE>` | Feature name (required) |
| `--method <METHOD>` | Method name (required) |
| `--params <JSON>` | JSON parameters for the method (optional) |

### Categories and Methods Reference

#### CONTEXT Category

**SystemState Feature:**
- `getHomeState` - 집 위치의 현재 상태
- `getWorkState` - 직장 위치의 현재 상태
- `getCommutePatternState` - 통근 패턴 상태
- `getSleepPatternState` - 수면 패턴 상태

**BasicDomain Feature:**
- `getExerciseContexts` - 운동 컨텍스트
- `getDriveContexts` - 운전 컨텍스트
- `getSleepContexts` - 수면 컨텍스트
- `getMoveContexts` - 이동 컨텍스트
- `getVisitedPlaces` - 방문한 장소
- `getFrequentlyVisitedPlaces` - 자주 방문한 장소

#### PERSONAL_DATA Category

**BasicDomain Feature:**
- `getMyProfile` - 사용자 프로필
- `getMyFamily` - 가족 정보

**LifestyleEntertainment Feature:**
- `getTopVideoList` - 상위 비디오 목록
- `getTopMusicList` - 상위 음악 목록
- `getTopArtists` - 상위 아티스트 목록

**LifestyleSocial Feature:**
- `getRecentContactPersonList` - 최근 연락한 사람
- `getMostContactPersonList` - 자주 연락하는 사람

See [crossdevice-context-cli.md](references/crossdevice-context-cli.md) for complete documentation.

## Multi Assistant

Coordinate multiple virtual assistants:

```bash
# List available assistants
tizen-multi-assistant-cli list

# Activate assistant
tizen-multi-assistant-cli activate --assistant bixby

# Deactivate assistant
tizen-multi-assistant-cli deactivate --assistant bixby

# Get assistant status
tizen-multi-assistant-cli status
```

See [tizen-multi-assistant-cli.md](references/tizen-multi-assistant-cli.md) for complete documentation.

## User Activity

Recognize and track user activities:

```bash
# Get current activity
tizen-ua-cli get-current-activity

# Get activity history
tizen-ua-cli get-history

# List supported activities
tizen-ua-cli list-activities

# Get default service
tizen-ua-cli get-default-service

# Get service info
tizen-ua-cli get-service --name ua.service.default
```

See [tizen-ua-cli.md](references/tizen-ua-cli.md) for complete documentation.

## When to Use

- User wants personalized recommendations based on mobile data
- User asks about their habits, preferences, or recent activities
- User wants to coordinate multiple virtual assistants
- User needs cross-device task continuity
- User wants to track or query their activity patterns

## When NOT to Use

- User wants to know what's currently on TV screen (use `tizen-context-cli` instead)
- User wants to search or play content (use `tizen-search-content-cli` / `tizen-play-content-cli` instead)
- User wants to control TV UI navigation (use `tizen-aurum-cli` instead)

## Common Use Cases

| User Request | Command Pattern |
|--------------|-----------------|
| "모바일에서 자주 본 영상 추천해줘" | `crossdevice-context-cli execute --category PERSONAL_DATA --feature LifestyleEntertainment --method getTopVideoList` |
| "집에 있어?" | `crossdevice-context-cli execute --category CONTEXT --feature SystemState --method getHomeState` |
| "내 프로필 정보 보여줘" | `crossdevice-context-cli execute --category PERSONAL_DATA --feature BasicDomain --method getMyProfile` |
| "어떤 어시스턴트 있어?" | `tizen-multi-assistant-cli list` |
| "지금 어떤 활동 중이야?" | `tizen-ua-cli get-current-activity` |

## Error handling

All CLI tools return exit codes:
- 0: Success
- Non-zero: Error (check error message for details)

Error codes for crossdevice-context-cli: `missing_argument`, `invalid_argument`, `unknown_command`, `app_control_error`

Use `--help` flag with any CLI tool for detailed usage information.