add_rules("mode.debug", "mode.release")

set_languages('c++23')
  
target('syntax-analyzer')
  add_files('src/*.cpp')
  set_toolchains('clang')
  add_ldflags('-static')
  after_build(function (target)
    os.cp("input/*", target:targetdir())
  end)