add_rules("mode.release","mode.debug")
target("sysyc")
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("include/")
    set_languages("c++17")