AM_CFLAGS = -Wall -Werror -export-dynamic @GTK_CFLAGS@ @GCONF_CFLAGS@ @LIBWNCK_CFLAGS@ @VFS_CFLAGS@

bin_PROGRAMS = multi-backgrounds-daemon

multi_backgrounds_daemon_SOURCES = \
	multi-backgrounds-daemon.c

multi_backgrounds_daemon_LDFLAGS = @GTK_LIBS@ @GCONF_LIBS@ @LIBWNCK_LIBS@ @VFS_LIBS@
