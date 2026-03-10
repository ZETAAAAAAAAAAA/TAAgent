---
name: ue-coding-workflow
description: This skill should be used when working with Unreal Engine C++ code changes, compiling the project, handling build errors, or adding new features to the MCP plugin. Trigger when the user asks to add new UE functionality, fix compilation errors, or modify C++ code.
---

# UE C++ Coding & Compilation Workflow

## Compilation Commands

```powershell
# Build the project
cd d:\CodeBuddy\rendering-mcp
powershell -ExecutionPolicy Bypass -File Build.ps1
```

Build logs are saved to `build_logs/build_YYYYMMDD_HHMMSS.log`.

## Common Compilation Issues

### 1. DLL Locked by UE Editor

**Error:**
```
fatal error LNK1104: cannot open file 'UnrealEditor-UnrealMCP.dll'
The process cannot access the file because it is being used by another process
```

**Solution:** Close UE Editor before compiling.

### 2. Header File Not Found

**Error:**
```
fatal error C1083: Cannot open include file: 'SomeHeader.h'
```

**Solution:**
1. Search UE source for the header: `dir /s /b "E:\UE\UE_5.7\Engine\Source\*SomeHeader*.h"`
2. Find the module containing the header
3. Add module to `UnrealMCP.Build.cs`:
   ```csharp
   PublicDependencyModuleNames.AddRange(new string[] { 
       "Core", "CoreUObject", "Engine", "TheModuleYouNeed" 
   });
   ```
4. For Internal APIs, add include path:
   ```csharp
   PublicIncludePaths.Add("Engine/Source/Runtime/SomeModule/Private");
   ```

### 3. API Method Not Found

**Error:**
```
error C2039: 'SomeMethod': is not a member of 'SomeClass'
```

**Solution:**
1. Search UE source for alternative implementations
2. Check if the method was renamed or moved in newer UE versions
3. Use similar patterns from other code in the project

## Design Principles

### MCP Tool Design

1. **Generic over Specific** - One generic tool > multiple specific tools
   - ✅ Use `get_assets(asset_class="X")` 
   - ❌ Don't create `get_X_assets()` for each type

2. **Reflection Driven** - Handle any type via UE reflection
   - Use `spawn_actor(actor_class)` not `spawn_light()`, `spawn_mesh()`

3. **Atomic Operations** - Complete operation in one call
   - Use `build_material_graph()` not multiple `add_node()` + `connect()`

4. **Batch Processing** - Reduce round-trips with batch tools

### Code Organization

```
mcps/unreal_render_mcp/tools/
├── __init__.py      # Export all tools
├── material.py      # Material operations
├── blueprint.py     # Blueprint operations
├── actor.py         # Actor operations
└── ...

plugins/unreal/UnrealMCP/RenderingMCP/Plugins/UnrealMCP/Source/UnrealMCP/
├── Public/Commands/
│   └── EpicUnrealMCP*Commands.h
└── Private/Commands/
    └── EpicUnrealMCP*Commands.cpp
```

## Adding New MCP Tool

### Step 1: Python Tool Definition

Create or edit file in `mcps/unreal_render_mcp/tools/`:

```python
@with_unreal_connection
def new_tool(param: str) -> Dict[str, Any]:
    """Tool description."""
    return send_command("new_command", {"param": param})
```

### Step 2: C++ Command Handler

Add to header file:
```cpp
TSharedPtr<FJsonObject> HandleNewCommand(const TSharedPtr<FJsonObject>& Params);
```

Add to cpp file:
```cpp
// In HandleCommand()
else if (CommandType == TEXT("new_command"))
{
    return HandleNewCommand(Params);
}

// Implementation
TSharedPtr<FJsonObject> FEpicUnrealMCP*Commands::HandleNewCommand(const TSharedPtr<FJsonObject>& Params)
{
    // ... implementation
    return ResultObj;
}
```

### Step 3: Register Tool

In `server.py`:
```python
from tools import new_tool
mcp.tool()(new_tool)
```

In `tools/__init__.py`:
```python
from module import new_tool
__all__ = [..., "new_tool"]
```

## UE Source Reference

- UE Source Location: `E:\UE\UE_5.7`
- API Headers: `Engine/Source/Runtime/*/Public/`
- Internal Headers: `Engine/Source/Runtime/*/Private/`

## After Adding New Feature

1. **Compile and Verify**
   ```powershell
   # Step 1: Compile
   cd d:\CodeBuddy\rendering-mcp
   powershell -ExecutionPolicy Bypass -File Build.ps1

   # Step 2: Start UE Editor (user can help with this)
   # The UE project is at: plugins/unreal/UnrealMCP/RenderingMCP/RenderingMCP.uproject

   # Step 3: Test MCP tool immediately
   # Use mcp_call_tool to verify the new functionality works
   ```

2. **Update Documentation**
   - Update this skill if new patterns emerge
   - Update project rules if new conventions established
   - Document any UE version-specific quirks

3. **Update Related Skills**
   - If adding Niagara features → update `ue-niagara-workflow`
   - If adding Material features → update `ue-material-workflow`
   - If adding Rendering features → update `ue-rendering-pipeline`

## Common UE 5.7 API Patterns

### Accessing Private Properties via Reflection

When a class uses `UCLASS(MinimalAPI)`, getter methods may not be exported. Use reflection:

```cpp
// Instead of: Obj->GetPrivateProperty()  // Link error!

// Use reflection:
if (FStrProperty* Prop = CastField<FStrProperty>(Obj->GetClass()->FindPropertyByName(TEXT("PrivateProperty"))))
{
    FString Value = Prop->GetPropertyValue_InContainer(Obj);
}
```

### Enum to String Conversion

```cpp
UEnum* MyEnum = StaticEnum<EMyEnumType>();
if (MyEnum)
{
    FString EnumName = MyEnum->GetNameStringByValue((int64)EnumValue);
}
```

### Inheritance in Cast Checks

Always check derived classes before base classes:

```cpp
// WRONG: CustomHlsl inherits from FunctionCall, this catches it first
if (UNiagaraNodeFunctionCall* FuncNode = Cast<UNiagaraNodeFunctionCall>(Node))
{ ... }
else if (UNiagaraNodeCustomHlsl* HlslNode = Cast<UNiagaraNodeCustomHlsl>(Node))  // Never reached!
{ ... }

// CORRECT: Check derived class first
if (UNiagaraNodeCustomHlsl* HlslNode = Cast<UNiagaraNodeCustomHlsl>(Node))
{ ... }
else if (UNiagaraNodeFunctionCall* FuncNode = Cast<UNiagaraNodeFunctionCall>(Node))
{ ... }
```
