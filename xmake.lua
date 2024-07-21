add_rules("mode.debug", "mode.release")

set_languages("c++23")
  
target("syntax-analyzer")
  add_files("src/*.cpp")
  add_includedirs("include")
  set_rundir("$(projectdir)")