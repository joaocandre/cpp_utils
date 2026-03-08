project('cpp_utils', 'cpp',
    version: '1.0.0',
    default_options : [
        'warning_level=2',
        'cpp_std=c++17'
    ],
    meson_version: '>=1.1'  # to include meson.options)
)

# look for dependencies
boost = dependency('Boost', required: false)
if not boost.found()
    message()
endif
tinyxml2 = dependency('tinyxml2', required: false)
if not boost.found()
    message()
endif
doxygen_dep = dependency('doxygen', required: get_option('gen_docs'))

inc = include_directories('include')
dep = declare_dependency(include_directories: inc)

meson.override_dependency('cpp_utils', dep)

summary({
    'Generating API docs': get_option('gen_docs'),
})


# @todo have meson generate documentation (?)
# @todo list same options