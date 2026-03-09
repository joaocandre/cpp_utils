project('cpp_utils', 'cpp',
    version: '1.0.0',
    default_options : [
        'warning_level=0',   # mute warnings
        'cpp_std=c++17'
    ],
    meson_version: '>=1.1',  # to include meson.options)
    # subproject_dir: '.subprojects',
    license: 'MIT',
    license_files: 'LICENSE'
)

# dependencies
boost = dependency('Boost', required: false)
if not boost.found()
    warning('Boost not found: installation is required to use thread utilities')
endif
tinyxml2 = dependency('tinyxml2', required: false)
if not boost.found()
    warning('tinyxml2 not found: installation is required for XML utilities')
endif
doxygen_dep = dependency('doxygen', required: get_option('gen_docs'))

# declare header files
headers = [
    'include/extra/chrono.hpp',
    'include/extra/cli.hpp',
    'include/extra/exception.hpp',
    'include/extra/math.hpp',
    'include/extra/stream.hpp',
    'include/extra/xml.hpp',
    'include/storage/boolean.hpp',
    'include/storage/cast_iterator.hpp',
    'include/storage/comparer.hpp',
    'include/storage/indexer.hpp',
    'include/storage/iterator.hpp',
    'include/storage/matrix.hpp',
    'include/storage/numeric.hpp',
    'include/storage/range_iterator.hpp',
    'include/storage/sorted.hpp',
    'include/storage/statistical.hpp',
    'include/storage/stream.hpp',
    'include/storage/subset.hpp',
    'include/storage/tagged.hpp',
    'include/storage/type_check.hpp',
    'include/storage/volume.hpp',
    'include/storage/wrap_iterator.hpp',
    'include/thread/basic_executor.hpp',
    'include/type/check.hpp',
    'include/type/details.hpp'
]
install_subdir('include')

# declare as dependency for consumer projects
dependencies = [boost_asio_dep, tinyxml2_dep]
include_dirs = include_directories('include')
cpp_utils_dep = declare_dependency(include_directories: include_dirs, dependencies: dependencies)
meson.override_dependency('cpp_utils', cpp_utils_dep)

# build API documentation
# @todo move to nested meson.build (?)
if has_deps and get_option('gen_docs')
    dependency('Doxygen')
    conf_data = configuration_data()
    conf_data.set('CMAKE_PROJECT_NAME', meson.project_name())
    conf_data.set('CMAKE_PROJECT_VERSION', meson.project_version())
    conf_data.set('CMAKE_PROJECT_BRIEF', 'C++ header-only library providing generic extensions to STL containers.')
    # conf_data.set('CMAKE_PROJECT_LOGO', 'doc/logo.jpg')
    conf_data.set('CMAKE_CURRENT_SOURCE_DIR', meson.project_source_root())
    configure_file(input: 'doc/Doxyfile.in', output: 'Doxyfile', configuration: conf_data)
    message(meson.project_build_root() + '/Doxyfile')
    # run_target(meson.project_name() + '_doc', command : ['doxygen', meson.project_build_root() + '/Doxyfile'], depends: mplibrary_lib)
    custom_target(meson.project_name() + '_doc',
                  command : ['doxygen', meson.project_build_root() + '/Doxyfile'],
                  output : ['.dummy'],
                  build_by_default: true)
    # @todo add install target for documentation
endif

# configuration summary
summary({
    'Generating API docs': get_option('gen_docs'),
})
