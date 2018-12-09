-- define target

target("sm")

    -- set kind
    set_kind("shared")

    -- add files
    add_files("memory.c")
	add_cxflags("-Wno-error=deprecated-declarations", "-fno-strict-aliasing")
	set_targetdir("$(plat)")