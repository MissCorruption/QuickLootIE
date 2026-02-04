set_xmakever("2.9.5")

-- Globals
PROJECT_NAME = "QuickLootIE"

-- Project
set_project(PROJECT_NAME)
set_version("3.4.1")
set_languages("cxx23")
set_license("mit")
set_warnings("allextra", "error")

-- Build options
option("auto_plugin_deployment", {default = false, description = "Copy the build output and addons to env:OutputDir."})
option("zip_to_dist", {default = true, description = "Zip the base mod and addons to their own 7z file in dist."})
option("aio_zip_to_dist", {default = false, description = "Zip the base mod and addons to a AIO 7z file in dist."})


-- Dependencies & Includes
-- https://github.com/xmake-io/xmake-repo/tree/dev    
add_requires("fmt", "magic_enum", "nlohmann_json", "xbyak")

includes("extern/CommonLibSSE-NG")

-- policies
set_policy("package.requires_lock", true)
set_policy("check.auto_ignore_flags", false)

-- rules
add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_mode("debug") then
    add_defines("DEBUG")
    set_optimize("none")
elseif is_mode("release") then
    add_defines("NDEBUG")
    set_optimize("fastest")
    set_symbols("debug")
end

set_config("skse_xbyak", true)
set_config("skyrim_se", true)
set_config("skyrim_ae", true)
set_config("skyrim_vr", true)

-- Target
target(PROJECT_NAME)
    -- Dependencies
    add_packages("fmt", "magic_enum", "nlohmann_json", "xbyak")

    -- CommonLibSSE
    add_deps("commonlibsse-ng")
    add_rules("commonlibsse-ng.plugin", {
        name = PROJECT_NAME,
        author = "Miss Corruption & AtomCrafty",
        description = "A fork of QuickLoot EE that adds new features, fixes bugs, and incorporates native compatibility for mods that previously needed patches. "
        }
    )

    -- Source files
    set_pcxxheader("src/PCH.h")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    add_includedirs("include")

    -- flags
    add_cxxflags(
        "cl::/cgthreads4",
        "cl::/diagnostics:caret",
        "cl::/DNOMINMAX",
        "cl::/external:W0",
        "cl::/fp:contract",
        "cl::/fp:except-",
        "cl::/guard:cf-",
        "cl::/Zc:enumTypes",
        "cl::/Zc:preprocessor",
        "cl::/Zc:templateScope",
        "cl::/utf-8"
        )
    -- flags (cl: warnings -> errors)
    add_cxxflags("cl::/we4715") -- `function` : not all control paths return a value
    -- flags (cl: disable warnings)
    add_cxxflags(
        "cl::/wd4068", -- unknown pragma 'clang'
        "cl::/wd4201", -- nonstandard extension used : nameless struct/union
        "cl::/wd4265" -- 'type': class has virtual functions, but its non-trivial destructor is not virtual; instances of this class may not be destructed correctly
        )
    -- Conditional flags
    if is_mode("debug") then
        add_cxxflags("cl::/bigobj")
    elseif is_mode("release") then
        add_cxxflags("cl::/Zc:inline", "cl::/JMC-", "cl::/Ob3")
    end

    -- Post Build 
    after_build(function (target)
        if has_config("auto_plugin_deployment") then
            local output_dir = os.getenv("OutputDir")
            if output_dir then
                local plugin_dir = path.join(output_dir, "SKSE/Plugins/")
                os.cp(target:targetfile(), plugin_dir)
                if target:targetfile() .. ".pdb" then
                    os.cp(target:targetfile() .. ".pdb", plugin_dir)
                end
                print("Deployed to: " .. plugin_dir)
            else
                print("Warning: AUTO_PLUGIN_DEPLOYMENT is enabled, but OutputDir is not set!")
            end
        end
    end)

target_end()
