-- Rome 2 dummy smoke suite.
-- Runs as soon as the campaign scripting.lua loads twdll.
-- Does not depend on Attila's events.FirstTickAfterWorldCreated table.

local function log(msg)
    if type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.Log) == "function" then
        twdll.core.Log(msg)
    else
        local f = io.open("twdll.log", "a")
        if f then
            f:write(tostring(msg) .. "\n")
            f:close()
        end
    end
end

local passed = 0
local failed = 0

local function assert_true(name, condition)
    if condition then
        passed = passed + 1
        log(string.format("[TEST]   assert '%s': OK", name))
        return true
    end
    failed = failed + 1
    log(string.format("[TEST]   assert '%s': FAILED", name))
    return false
end

log("[TEST] Rome 2 dummy smoke")
assert_true("twdll table", type(twdll) == "table")
assert_true("twdll.core table", type(twdll) == "table" and type(twdll.core) == "table")
assert_true("twdll.core.Log", type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.Log) == "function")
assert_true("twdll.core.GameBuild", type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.GameBuild) == "function")
assert_true("twdll.core.GetBuildSha", type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.GetBuildSha) == "function")

local game_build = nil
if type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.GameBuild) == "function" then
    game_build = twdll.core.GameBuild()
end
assert_true("GameBuild == Rome2", game_build == "Rome2")
log("[TEST] GameBuild=" .. tostring(game_build))

local sha = nil
if type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.GetBuildSha) == "function" then
    sha = twdll.core.GetBuildSha()
end
assert_true("GetBuildSha non-empty", type(sha) == "string" and #sha > 0)
log("[TEST] GetBuildSha=" .. tostring(sha))

-- Multi-arg Log exercises getfield/type/pushvalue/pcall/settop via tostring.
local multi_ok = pcall(function()
    twdll.core.Log("[TEST] multi-arg Log", 42, true, nil)
end)
assert_true("multi-arg Log", multi_ok)

assert_true("ProbeScriptInterfaces", type(twdll.core.ProbeScriptInterfaces) == "function")
local hits = nil
local probe_ok, probe_err = pcall(function()
    hits = twdll.core.ProbeScriptInterfaces()
end)
assert_true("ProbeScriptInterfaces call", probe_ok)
if not probe_ok then
    log("[TEST] ProbeScriptInterfaces error: " .. tostring(probe_err))
end
assert_true("ProbeScriptInterfaces table", type(hits) == "table")
if type(hits) == "table" then
    assert_true("FACTION_SCRIPT_INTERFACE", hits.FACTION_SCRIPT_INTERFACE == true)
    assert_true("CHARACTER_SCRIPT_INTERFACE", hits.CHARACTER_SCRIPT_INTERFACE == true)
    assert_true("UNIT_SCRIPT_INTERFACE", hits.UNIT_SCRIPT_INTERFACE == true)
    assert_true("MILITARY_FORCE_SCRIPT_INTERFACE", hits.MILITARY_FORCE_SCRIPT_INTERFACE == true)
end

log(string.format("[TEST] PASSED: %d FAILED: %d SKIPPED: 0", passed, failed))
