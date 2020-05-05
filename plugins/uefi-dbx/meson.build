cargs = ['-DG_LOG_DOMAIN="FuPluginUefiDbx"']

shared_module('fu_plugin_uefi_dbx',
  fu_hash,
  sources : [
    'fu-plugin-uefi-dbx.c',
    'fu-uefi-dbx-common.c',
    'fu-uefi-dbx-file.c',
  ],
  include_directories : [
    root_incdir,
    fwupd_incdir,
    fwupdplugin_incdir,
  ],
  install : true,
  install_dir: plugin_dir,
  link_with : [
    fwupd,
    fwupdplugin,
  ],
  c_args : cargs,
  dependencies : [
    plugin_deps,
  ],
)

if get_option('tests')
  testdatadir = join_paths(meson.current_source_dir(), 'tests')
  cargs += '-DTESTDATADIR="' + testdatadir + '"'
  e = executable(
    'uefi-dbx-self-test',
    fu_hash,
    sources : [
      'fu-self-test.c',
      'fu-uefi-dbx-common.c',
      'fu-uefi-dbx-file.c',
    ],
    include_directories : [
      root_incdir,
      fwupd_incdir,
      fwupdplugin_incdir,
    ],
    dependencies : [
      plugin_deps,
    ],
    link_with : [
      fwupd,
      fwupdplugin,
    ],
    c_args : cargs
  )
  test('uefi_dbx_file-self-test', e)
endif
