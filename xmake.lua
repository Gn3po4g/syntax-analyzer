add_rules("mode.release", "mode.debug")

set_languages('c++latest')

target('syntax-analyzer')
  set_kind('binary')
  add_files('src/*.cpp')
  add_cxxflags('-stdlib=libc++')
  add_ldflags('-stdlib=libc++')
  set_toolchains('clang')
  after_build(function (target)
    os.cp("input/*", target:targetdir())
  end)