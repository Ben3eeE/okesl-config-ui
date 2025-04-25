project('okesl-config-ui', 'cpp', version: '1.0.0', default_options: ['cpp_std=c++17'])

src = files(
  'src/main.cpp',
  'src/CvarManager.cpp',
  'src/ControlsManager.cpp'
)

inc = include_directories('src', 'include')

cxx = meson.get_compiler('cpp')
sdl2_proj = subproject('sdl2', default_options: ['default_library=static'])
sdl2_dep = sdl2_proj.get_variable('sdl2_dep')
imgui_proj = subproject('imgui', default_options: ['default_library=static'])
imgui_dep = imgui_proj.get_variable('imgui_dep')
nlohmann_json_proj = subproject('nlohmann_json')
nlohmann_json_dep = nlohmann_json_proj.get_variable('nlohmann_json_dep')

deps = [sdl2_dep, imgui_dep, nlohmann_json_dep]

if host_machine.system() == 'windows'
  sdl2main_dep = sdl2_proj.get_variable('sdl2main_dep')
  deps += sdl2main_dep
  add_project_link_arguments('-static', language: 'cpp')
endif

link_args = []


executable(
    'okesl-config-ui',
    sources: src,
    include_directories: inc,
    dependencies: deps,
    win_subsystem: 'windows',
    link_args: link_args,
)
