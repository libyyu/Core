-- define target
target("test")

    -- set kind
    set_kind("binary")

    -- add files
    add_files("test.cpp")

    set_languages("cxx11")

    add_includedirs("../lua51/src")
    add_linkdirs("libs/$(plat)")
    add_links("lua51")
