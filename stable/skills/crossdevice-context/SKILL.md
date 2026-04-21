---
name: crossdevice-context
description: Retrieve personalized context and data from nearby devices (primarily mobile) via D2D communication. Use when the user requests personalized information based on preferences, habits, recent activities, or cross-device task continuity. Subcommand required (list-tools, execute).
argument-hint: "[list-tools|execute --category <CATEGORY> --feature <FEATURE> --method <METHOD>]"
user-invocable: true
allowed-tools: true
---

# Cross-Device Context Tool

Retrieve personalized context and data from nearby devices (primarily mobile) via D2D (Device-to-Device) communication to provide a personalized user experience. This tool enables access to user's personal habits, physical states, location history, media consumption, and other context information stored on peer devices.

**Prerequisite:** `com.samsung.tizen.crossdevice-context` service must be running and a mobile device must be connected.

## Intent-Based Workflow

| User says | Action | Example |
|---|---|---|
| "모바일에서 자주 본 영상 추천해줘" | `execute getTopVideoList` | `crossdevice-context-cli execute --category PERSONAL_DATA --feature LifestyleEntertainment --method getTopVideoList` |
| "내가 좋아하는 음악 뭐야?" | `execute getTopMusicList` | `crossdevice-context-cli execute --category PERSONAL_DATA --feature LifestyleEntertainment --method getTopMusicList` |
| "지금 운동 중이야?" | `execute getExerciseContexts` | `crossdevice-context-cli execute --category CONTEXT --feature BasicDomain --method getExerciseContexts` |
| "집에 있어?" | `execute getHomeState` | `crossdevice-context-cli execute --category CONTEXT --feature SystemState --method getHomeState` |
| "최근 방문한 장소 알려줘" | `execute getVisitedPlaces` | `crossdevice-context-cli execute --category CONTEXT --feature BasicDomain --method getVisitedPlaces` |
| "내 프로필 정보 보여줘" | `execute getMyProfile` | `crossdevice-context-cli execute --category PERSONAL_DATA --feature BasicDomain --method getMyProfile` |
| "어떤 도구들 사용 가능해?" | `list-tools` | `crossdevice-context-cli list-tools` |

## When to Use This Tool

### 1. Retrieval of Private Personal Context
Use this tool when the user's request pertains to personal habits, physical states, or private information (e.g., location history, user profile) that is typically managed on a personal mobile device rather than a shared TV.

### 2. Provision of Cross-Device Task Continuity
Use this tool to provide a seamless experience across devices, such as identifying the user's recent activities on mobile (e.g., media consumption, social trends) to resume tasks or offer relevant "what's next" suggestions on the TV.

## When NOT to Use This Tool

- User wants to know what's currently on TV screen (use `tizen-context-cli` instead)
- User wants to search or play content (use `tizen-search-content-cli` / `tizen-play-content-cli` instead)
- User wants to control TV UI navigation (use `tizen-aurum-cli` instead)

## CLI Usage

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

### Global Options

| Option | Description |
|---|---|
| `-h`, `--help` | Show help message |
| `--schema` | Print MCP-compatible JSON Schema Draft 7 and exit |
| `--help-actions` | Show available App Control actions |
| `--help-categories` | Show available categories, features, and methods |

### Execute Options

| Option | Description |
|---|---|
| `--category <CATEGORY>` | Tool category: `CONTEXT` or `PERSONAL_DATA` (required) |
| `--feature <FEATURE>` | Feature name (required) |
| `--method <METHOD>` | Method name (required) |
| `--params <JSON>` | JSON parameters for the method (optional) |

Exit codes: `0` success, `1` runtime error, `2` usage error.

## Recommended Workflow

### Scenario: "Recommend content similar to what I've been watching frequently on mobile lately"

#### Step 1: Discover Available Tools (Optional)
If unsure which tools are available, list all tools first:

```bash
crossdevice-context-cli list-tools
```

Result:
```json
{
  "status": "success",
  "tool": "crossdevice-context-cli",
  "message": "All tools retrieved successfully",
  "data": {
    "tools": [
      {
        "category": "CONTEXT",
        "feature": "BasicDomain",
        "method": "getDriveContexts"
      },
      {
        "category": "PERSONAL_DATA",
        "feature": "LifestyleEntertainment",
        "method": "getTopVideoList"
      }
    ],
    "count": 2
  }
}
```

#### Step 2: Execute Relevant Tool
Based on the user's context about "watching video content", execute `getTopVideoList`:

```bash
crossdevice-context-cli execute --category PERSONAL_DATA --feature LifestyleEntertainment --method getTopVideoList
```

Result:
```json
{
  "status": "success",
  "tool": "crossdevice-context-cli",
  "message": "Tool executed successfully",
  "data": {
    "category": "PERSONAL_DATA",
    "feature": "LifestyleEntertainment",
    "method": "getTopVideoList",
    "result": [
      "Video(title=[FIFA 클럽월드컵] 플루미넨시 vs 울산 HD FC, artist=쿠팡플레이 스포츠, preferences=Bundle[{count=6}])",
      "Video(title=손흥민 미국 최강 공격 듀오, artist=흰수염고래스포츠, preferences=Bundle[{count=5}])"
    ]
  }
}
```

#### Step 3: Use Retrieved Context
Use the retrieved context data (e.g., video titles, genres) to perform content search using `tizen-search-content-cli` and display results via `tizen-hud-cli`.

## User Query → Tool Selection Guide

### CONTEXT Category

#### SystemState Feature
| User Query Pattern | Method | Example |
|---|---|---|
| "집에 있어?", "집이야?" | `getHomeState` | "지금 집에 있어?" → getHomeState |
| "회사야?", "직장이야?" | `getWorkState` | "지금 회사야?" → getWorkState |
| "운전 중이야?", "차 몰고 있어?" | `getCommutePatternState` | "운전 중이야?" → getCommutePatternState |
| "자고 있어?", "수면 상태야?" | `getSleepPatternState` | "지금 자고 있어?" → getSleepPatternState |

#### BasicDomain Feature
| User Query Pattern | Method | Example |
|---|---|---|
| "운동 중이야?", "운동하고 있어?" | `getExerciseContexts` | "지금 운동 중이야?" → getExerciseContexts |
| "운전 중이야?", "차 몰고 있어?" | `getDriveContexts` | "운전 중이야?" → getDriveContexts |
| "자고 있어?", "수면 중이야?" | `getSleepContexts` | "지금 자고 있어?" → getSleepContexts |
| "이동 중이야?", "움직이고 있어?" | `getMoveContexts` | "지금 이동 중이야?" → getMoveContexts |
| "방문한 장소", "갔던 곳" | `getVisitedPlaces` | "최근 방문한 장소 알려줘" → getVisitedPlaces |
| "자주 가는 곳", "단골 장소" | `getFrequentlyVisitedPlaces` | "자주 가는 곳 어디야?" → getFrequentlyVisitedPlaces |

### PERSONAL_DATA Category

#### BasicDomain Feature
| User Query Pattern | Method | Example |
|---|---|---|
| "내 프로필", "내 정보" | `getMyProfile` | "내 프로필 정보 보여줘" → getMyProfile |
| "가족 정보", "가족 구성원" | `getMyFamily` | "가족 누구 있어?" → getMyFamily |

#### LifestyleEntertainment Feature
| User Query Pattern | Method | Example |
|---|---|---|
| "최근 본 영상", "자주 보는 비디오" | `getTopVideoList` | "최근 자주 본 영상 추천해줘" → getTopVideoList |
| "최근 들은 음악", "좋아하는 노래" | `getTopMusicList` | "내가 좋아하는 음악 뭐야?" → getTopMusicList |
| "좋아하는 가수", "아티스트 취향" | `getTopArtists` | "내가 좋아하는 가수 누구야?" → getTopArtists |
| "요일별 활동" | `rankDayOfWeek` | "요일별 미디어 활동 알려줘" → rankDayOfWeek |
| "시간대별 활동" | `rankTimeOfDay` | "시간대별 미디어 활동 알려줘" → rankTimeOfDay |

#### LifestyleHealthAndWellness Feature
| User Query Pattern | Method | Example |
|---|---|---|
| "운동 기록", "건강 상태" | `getFrequentExercisesByMonth` | "이번 주 운동 기록 보여줘" → getFrequentExercisesByMonth |
| "가장 많이 한 운동" | `getMostFrequentExercises` | "내가 제일 많이 한 운동 뭐야?" → getMostFrequentExercises |

#### LifestyleSocial Feature
| User Query Pattern | Method | Example |
|---|---|---|
| "최근 연락한 사람" | `getRecentContactPersonList` | "최근 연락한 친구 누구야?" → getRecentContactPersonList |
| "자주 연락하는 사람" | `getMostContactPersonList` | "가장 자주 연락하는 사람 누구야?" → getMostContactPersonList |

#### Event Feature
| User Query Pattern | Method | Example |
|---|---|---|
| "일정", "이벤트", "캘린더" | `queryEvents` | "오늘 일정 있어?" → queryEvents |

#### Interest Feature
| User Query Pattern | Method | Example |
|---|---|---|
| "내 관심사", "좋아하는 것" | `getInterests` | "내 관심사가 뭐야?" → getInterests |

#### Travel Feature
| User Query Pattern | Method | Example |
|---|---|---|
| "여행 계획", "여행 일정" | `searchTravelPlan` | "예정된 여행 있어?" → searchTravelPlan |

## Categories, Features, and Methods Reference

### CONTEXT Category
Tools for retrieving current user context and state information.

#### SystemState Feature
| Method | Description |
|--------|-------------|
| `getBehaviorEngineState` | Behavior Inference Engine의 현재 상태를 반환합니다. (1: 에러, 2: 준비중, 3: 정상) |
| `getCommutePatternState` | 통근 패턴의 현재 상태를 반환합니다. |
| `getHomeState` | 집 위치의 현재 상태를 반환합니다. |
| `getSleepPatternState` | 수면 패턴의 현재 상태를 반환합니다. |
| `getSpatialEngineState` | Spatial Inference Engine의 현재 상태를 반환합니다. |
| `getWorkState` | 직장 위치의 현재 상태를 반환합니다. |

#### BasicDomain Feature
| Method | Description |
|--------|-------------|
| `getAggregateSleepPatternContext` | 수면 패턴 주간 컨텍스트 정보를 반환합니다. |
| `getDriveContexts` | 지정된 시간 범위 내의 운전 컨텍스트 목록을 반환합니다. |
| `getExerciseContexts` | 지정된 시간 범위 내의 운동 컨텍스트 목록을 반환합니다. |
| `getExercisePatternContext` | 운동 패턴 컨텍스트 정보를 반환합니다. |
| `getFrequentlyVisitedPlaces` | 자주 방문한 장소 목록을 반환합니다. |
| `getGetOffWorkPatternContext` | 퇴근 패턴 컨텍스트 정보를 반환합니다. |
| `getGoToWorkPatternContext` | 출근 패턴 컨텍스트 정보를 반환합니다. |
| `getInactivityPatternContext` | 비활동 패턴 컨텍스트 정보를 반환합니다. |
| `getLastModifiedTimestampOfCandidatePhoneNumbers` | 후보 전화번호의 마지막 수정 타임스탬프를 반환합니다. |
| `getMoveContexts` | 지정된 시간 범위 내의 이동 컨텍스트 목록을 반환합니다. |
| `getMoveDetectingDevice` | 이동 컨텍스트의 현재 감지 장치를 반환합니다. |
| `getMoveState` | 이동 컨텍스트의 현재 상태를 반환합니다. |
| `getPeriodicallyVisitedAreas` | 주기적으로 방문한 지역 목록을 반환합니다. |
| `getPeriodicallyVisitedPlaces` | 주기적으로 방문한 장소 목록을 반환합니다. |
| `getSleepContexts` | 지정된 시간 범위 내의 수면 컨텍스트 목록을 반환합니다. |
| `getSleepPatternContext` | 수면 패턴 컨텍스트 정보를 반환합니다. |
| `getVisitedAreas` | 방문한 지역 목록을 반환합니다. |
| `getVisitedPlaces` | 방문한 장소 목록을 반환합니다. |

### PERSONAL_DATA Category
Tools for retrieving personal user data.

#### BasicDomain Feature
| Method | Description |
|--------|-------------|
| `getMyProfile` | 사용자의 프로필 정보를 반환합니다. |
| `getMyFamily` | 사용자의 가족 정보를 반환합니다. |

#### LifestyleEntertainment Feature
| Method | Description |
|--------|-------------|
| `getTopMusicList` | 상위 음악 목록을 반환합니다. |
| `getTopVideoList` | 상위 비디오 목록을 반환합니다. |
| `getTopArtists` | 상위 아티스트 목록을 반환합니다. |
| `rankDayOfWeek` | 요일별 랭킹을 반환합니다. |
| `rankTimeOfDay` | 시간대별 랭킹을 반환합니다. |

#### LifestyleHealthAndWellness Feature
| Method | Description |
|--------|-------------|
| `getFrequentExercisesByMonth` | 월별 빈번한 운동 목록을 반환합니다. |
| `getMostFrequentExercises` | 가장 빈번한 운동 목록을 반환합니다. |

#### LifestyleSocial Feature
| Method | Description |
|--------|-------------|
| `getMostFrequentExercises` | 소셜 기반 가장 빈번한 운동 목록을 반환합니다. |
| `getMostContactPersonList` | 가장 자주 연락한 사람 목록을 반환합니다. |
| `getRecentContactPersonList` | 최근 연락한 사람 목록을 반환합니다. |

#### Event Feature
| Method | Description |
|--------|-------------|
| `getEventService` | 이벤트 서비스를 초기화하고 반환합니다. |
| `queryEvents` | 이벤트를 조회합니다. |

#### Interest Feature
| Method | Description |
|--------|-------------|
| `getInterests` | 사용자의 관심사 목록을 반환합니다. |

#### Travel Feature
| Method | Description |
|--------|-------------|
| `searchTravelPlan` | 여행 계획을 검색합니다. |

## Output

All output is a JSON envelope on stdout. Use `--schema` for the full machine-readable schema.

### list-tools

```json
{
  "status": "success",
  "tool": "crossdevice-context-cli",
  "message": "All tools retrieved successfully",
  "data": {
    "tools": [
      {
        "category": "CONTEXT",
        "feature": "SystemState",
        "method": "getHomeState"
      },
      {
        "category": "PERSONAL_DATA",
        "feature": "LifestyleEntertainment",
        "method": "getTopVideoList"
      }
    ],
    "count": 2
  }
}
```

### execute

```json
{
  "status": "success",
  "tool": "crossdevice-context-cli",
  "message": "Tool executed successfully",
  "data": {
    "category": "PERSONAL_DATA",
    "feature": "LifestyleEntertainment",
    "method": "getTopVideoList",
    "result": [
      "Video(title=..., artist=..., preferences=...)",
      "Video(title=..., artist=..., preferences=...)"
    ]
  }
}
```

### Error

```json
{
  "status": "error",
  "tool": "crossdevice-context-cli",
  "code": "app_control_error",
  "message": "Failed to communicate with CrossdeviceContext. Device may not be connected.",
  "data": null
}
```

Error codes: `missing_argument`, `invalid_argument`, `unknown_command`, `app_control_error`.

## Error Handling

### CLI Tool Not Found
When `crossdevice-context-cli` is not installed or not available in PATH:

```bash
$ crossdevice-context-cli list-tools
bash: crossdevice-context-cli: command not found
```

**Action:** Inform the user that the Cross-Device Context feature is not available and suggest checking the installation.

### Device Not Connected
When the mobile device is not connected or unreachable:

```json
{
  "status": "error",
  "tool": "crossdevice-context-cli",
  "code": "app_control_error",
  "message": "Failed to send launch request. Is crossdevice-context service running?"
}
```

**Action:** Inform the user that the mobile device may not be connected and suggest checking device pairing.

### Method Not Implemented
Some methods may not be implemented in the ContextEngine SDK:

```json
{
  "status": "success",
  "tool": "crossdevice-context-cli",
  "message": "Tool executed successfully",
  "data": {
    "result": {
      "status": "not_implemented",
      "method": "getDriveContexts",
      "message": "Method not yet implemented in ContextEngine SDK"
    }
  }
}
```

**Action:** Inform the user that the requested feature is not yet available.

### Error Codes Reference

| Code | Description | Recovery Action |
|------|-------------|-----------------|
| `missing_argument` | Required argument not provided | Supply the missing argument |
| `invalid_argument` | Invalid parameter value | Correct the parameter value |
| `unknown_command` | Unknown subcommand | Use `list-tools` or `execute` |
| `app_control_error` | Failed to communicate with service | Check device connection |

## Related Tools

This tool is typically used in combination with other tools:

| Tool | Purpose | Example Usage |
|------|---------|---------------|
| `tizen-search-content-cli` | Search content based on retrieved context | Search for video titles from getTopVideoList |
| `tizen-hud-cli` | Present results to user | Show recommendations on TV HUD |
| `tizen-play-content-cli` | Play selected content | Launch content on TV |

## Examples

```bash
# List all available tools
crossdevice-context-cli list-tools

# Get home state
crossdevice-context-cli execute --category CONTEXT --feature SystemState --method getHomeState

# Get top video list
crossdevice-context-cli execute --category PERSONAL_DATA --feature LifestyleEntertainment --method getTopVideoList

# Get visited places with parameters
crossdevice-context-cli execute --category CONTEXT --feature BasicDomain --method getVisitedPlaces --params '{"limit":10}'

# Get user profile
crossdevice-context-cli execute --category PERSONAL_DATA --feature BasicDomain --method getMyProfile

# Get user interests
crossdevice-context-cli execute --category PERSONAL_DATA --feature Interest --method getInterests

# Search travel plan
crossdevice-context-cli execute --category PERSONAL_DATA --feature Travel --method searchTravelPlan

# Print JSON schema for machine discovery
crossdevice-context-cli --schema

# Show available actions
crossdevice-context-cli --help-actions

# Show available categories, features, and methods
crossdevice-context-cli --help-categories