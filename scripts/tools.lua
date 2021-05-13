
local TOOL_DIR = path.join(SRC_DIR, "tools")

function add_tool (link, name, dir)

    if dir == nil then
        dir = name
    end

    project (name)
        uuid (os.uuid(name))
        kind "ConsoleApp"

        links {
            link,
        }

        files {
            path.join(SRC_DIR, "*.h"),
            path.join(SRC_DIR, "*.cpp"),
            path.join(TOOL_DIR, "tools_main.cpp"),
            path.join(TOOL_DIR, dir, "**.h"),
            path.join(TOOL_DIR, dir, "**.cpp"),
            path.join(TOOL_DIR, dir, "*.natvis"),
        }

        includedirs {
            path.join(SRC_DIR),
            path.join(TOOL_DIR, dir),
            path.join(FMT_DIR, 'include'),
        }

        flags {
            "Cpp17",
        }

        defaultConfigurations()

end

group "tools"
    add_tool ({'ghoulies', 'kameo'}, 'hashlookup')

    configuration {}
       files {
            path.join(SRC_DIR, "ghoulies", "hashutils.h"),
            path.join(SRC_DIR, "kameo", "hashutils.h"),
       }

    add_tool ('ghoulies', 'bundleinfo')
    add_tool ('ghoulies', 'locinfo')
    add_tool ('ghoulies', 'demandinfo')
    add_tool ('ghoulies', 'ghouliedump')

    configuration {}
       files {
            path.join(SRC_DIR, "ghoulies", "hashutils.h"),
            path.join(SRC_DIR, "img", "img.h"),
       }

        links {
            "img",
        }

    add_tool ('kameo', 'kameo_app', 'kameo')
    add_tool ('conker_lr', 'conker_lr_app', 'conker_lr')
    add_tool ('banjokazooie_nb', 'banjokazooie_nb_app', 'banjokazooie_nb')
    add_tool ('pinata', 'pinata_app', 'pinata')
    add_tool ('perfectdark', 'perfectdark_app', 'perfectdark')

    add_tool ('onevs', 'onevs_app', 'onevsdump')

    configuration {}
       files {
            path.join(SRC_DIR, "img", "img.h"),
       }

        links {
            "img",
        }


    add_tool ('xzp', 'xzp_dump', 'xzpdump')


