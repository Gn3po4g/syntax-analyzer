add_rules("mode.debug", "mode.release")

set_languages('c++latest')
set_toolchains('clang')
set_runtimes("c++_static")
add_ldflags("-lc++abi")

target('syntax-analyzer', { files = 'src/*.cpp' })
  after_build(function (target)
    os.cp("input/*", target:targetdir())
  end)