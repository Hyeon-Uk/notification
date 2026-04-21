# crossdevice-context-cli
**Description**: Query cross-device context information from connected devices via CrossdeviceContext using App Control.

## App Information
- **App ID**: `com.samsung.tizen.crossdevice-context`

## Usage
```
crossdevice-context-cli <subcommand> [options]
crossdevice-context-cli --schema
crossdevice-context-cli --help-actions
crossdevice-context-cli --help-categories
```

## Options
| Option | Description |
|--------|-------------|
| `--schema` | Output JSON schema for MCP integration and pipeline automation |
| `--help-actions` | Show available actions |
| `--help-categories` | Show available categories, features, and methods |

## Subcommands
| Subcommand | Arguments | Description |
|---|---|---|
| `list-tools` | | List all available tools |
| `execute` | `--category --feature --method [--params]` | Execute a tool with category, feature, method |

## Actions
| Action | String |
|--------|--------|
| GET_ALL_TOOLS | `com.samsung.tizen.crossdevice-context.GET_ALL_TOOLS` |
| EXECUTE_TOOL | `com.samsung.tizen.crossdevice-context.EXECUTE_TOOL` |

## Categories, Features, and Methods

### CONTEXT Category

#### SystemState Feature
| Method |
|--------|
| `getBehaviorEngineState` |
| `getCommutePatternState` |
| `getHomeState` |
| `getSleepPatternState` |
| `getSpatialEngineState` |
| `getWorkState` |

#### BasicDomain Feature
| Method |
|--------|
| `getAggregateSleepPatternContext` |
| `getDriveContexts` |
| `getExerciseContexts` |
| `getExercisePatternContext` |
| `getFrequentlyVisitedPlaces` |
| `getGetOffWorkPatternContext` |
| `getGoToWorkPatternContext` |
| `getInactivityPatternContext` |
| `getLastModifiedTimestampOfCandidatePhoneNumbers` |
| `getMoveContexts` |
| `getMoveDetectingDevice` |
| `getMoveState` |
| `getPeriodicallyVisitedAreas` |
| `getPeriodicallyVisitedPlaces` |
| `getSleepContexts` |
| `getSleepPatternContext` |
| `getVisitedAreas` |
| `getVisitedPlaces` |

### PERSONAL_DATA Category

#### BasicDomain Feature
| Method |
|--------|
| `getMyProfile` |
| `getMyFamily` |

#### LifestyleEntertainment Feature
| Method |
|--------|
| `getTopMusicList` |
| `getTopVideoList` |
| `getTopArtists` |
| `rankDayOfWeek` |
| `rankTimeOfDay` |

#### LifestyleHealthAndWellness Feature
| Method |
|--------|
| `getFrequentExercisesByMonth` |
| `getMostFrequentExercises` |

#### LifestyleSocial Feature
| Method |
|--------|
| `getMostFrequentExercises` |
| `getMostContactPersonList` |
| `getRecentContactPersonList` |

#### Event Feature
| Method |
|--------|
| `getEventService` |
| `queryEvents` |

#### Interest Feature
| Method |
|--------|
| `getInterests` |

#### Travel Feature
| Method |
|--------|
| `searchTravelPlan` |

## Output Format

### Success Response
```json
{
  "status": "success",
  "tool": "crossdevice-context-cli",
  "message": "Tool executed successfully",
  "data": {
    "result": "..."
  }
}
```

### Error Response
```json
{
  "status": "error",
  "tool": "crossdevice-context-cli",
  "code": "missing_argument",
  "message": "Category is required. Use --category <category>",
  "data": null
}
```

## Error Codes
| Code | Description |
|------|-------------|
| `missing_argument` | Required argument not provided |
| `invalid_argument` | Invalid parameter value |
| `unknown_command` | Unknown subcommand |
| `app_control_error` | Failed to communicate with CrossdeviceContext |

## Examples
```bash
# List all tools
crossdevice-context-cli list-tools

# Execute CONTEXT SystemState getHomeState
crossdevice-context-cli execute --category CONTEXT --feature SystemState --method getHomeState

# Execute PERSONAL_DATA BasicDomain getMyProfile
crossdevice-context-cli execute --category PERSONAL_DATA --feature BasicDomain --method getMyProfile

# Execute with parameters
crossdevice-context-cli execute --category CONTEXT --feature BasicDomain --method getVisitedPlaces --params '{"limit":10}'

# Show available actions
crossdevice-context-cli --help-actions

# Show available categories, features, and methods
crossdevice-context-cli --help-categories

# Output JSON schema
crossdevice-context-cli --schema
```

## JSON Schema

### Output Envelope
All subcommands return a consistent envelope structure:

| Field | Type | Description |
|-------|------|-------------|
| `status` | string | `"success"` or `"error"` |
| `tool` | string | Always `"crossdevice-context-cli"` |
| `message` | string | Human-readable summary |
| `code` | string | Error code (only on error) |
| `data` | object/null | Response data |

### Subcommand-specific data structures

#### list-tools
```json
{
  "tools": [
    {
      "category": "string",
      "feature": "string",
      "method": "string"
    }
  ],
  "count": "integer"
}
```

#### execute
```json
{
  "category": "string",
  "feature": "string",
  "method": "string",
  "result": "object"
}
## INSTALLATION

This tool is installed as part of the **tizen-tools** package:

```bash
# Install tizen-tools package
sdb shell pkgcmd -i tizen-tools
```

After installation, the tool is available at:
`/opt/usr/home/owner/.local/bin/crossdevice-context-cli`
