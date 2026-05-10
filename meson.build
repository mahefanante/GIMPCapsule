project('gimp-dynamic-inkscape', 'c',
  version: '6.2.1',
  meson_version: '>=0.50.0'
)

i18n = import('i18n')
gnome = import('gnome')

# Dependencies
libgimp_dep    = dependency('gimp-3.0')
libgimpui_dep  = dependency('gimpui-3.0')
gtk3_dep       = dependency('gtk+-3.0')
gegl_dep       = dependency('gegl-0.4')
json_glib_dep  = dependency('json-glib-1.0')
zlib_dep       = dependency('zlib')

deps = [
  libgimp_dep,
  libgimpui_dep,
  gtk3_dep,
  gegl_dep,
  json_glib_dep,
  zlib_dep
]

# Source files
sources = [
  'main.c',
  'metadata.c'
]

# Executable
executable('gimp-dynamic-inkscape',
  sources,
  dependencies: deps,
  install: true,
  install_dir: get_option('prefix')
)
