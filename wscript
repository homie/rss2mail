# vim:ft=python
#from Configure import conf
VERSION = '0.0.1'
APPNAME = 'rss2mail'

srcdir = '.'
builddir = 'output'

def configure(conf):
    conf.check_tool('gcc')

    conf.check_cfg(package='libxml-2.0', uselib_store='LIBXML', args='--cflags --libs')
    conf.check_cfg(package='libcurl', uselib_store='LIBCURL', args='--cflags --libs')

def build(bld):
    subdirs = 'src'
    bld.add_subdirs(subdirs)
