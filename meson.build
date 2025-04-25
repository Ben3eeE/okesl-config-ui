project('okesl-config-ui', 'cpp', version: '1.0.0', default_options: ['c_std=c++11'])

src = files(
  'src/main.cpp',
  'src/CvarManager.cpp',
  'src/ControlsManager.cpp'
)

inc = include_directories('src', 'include')

cxx = meson.get_compiler('cpp')
sdl2_proj = subproject('sdl2')
sdl2_dep = sdl2_proj.get_variable('sdl2_dep')
imgui_proj = subproject('imgui')
imgui_dep = imgui_proj.get_variable('imgui_dep')
nlohmann_json_proj = subproject('nlohmann_json')
nlohmann_json_dep = nlohmann_json_proj.get_variable('nlohmann_json_dep')

deps = [sdl2_dep, imgui_dep, nlohmann_json_dep]

link_args = []
if build_machine.system() == 'windows'
    link_args = ['/ENTRY:mainCRTStartup']
endif

executable(
    'okesl-config-ui',
    sources: src,
    include_directories: inc,
    dependencies: deps,
    win_subsystem: 'windows',
    link_args: link_args,
)
