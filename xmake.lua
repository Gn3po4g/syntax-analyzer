add_rules("mode.debug", "mode.release")

set_languages("c++23")
  
target("syntax-analyzer", { files = "src/*.cpp" })
set_rundir("$(projectdir)")
  add_includedirs("include")
  add_ldflags("-fuse-ld=mold")