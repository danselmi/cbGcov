AM_CPPFLAGS = $(WX_CXXFLAGS) \
		-I$(top_srcdir)/src/include \
		-I$(top_srcdir)/src/sdk/wxscintilla/include


pluginlibdir = $(pkglibdir)/plugins

pluginlib_LTLIBRARIES = libcbGcov.la

libcbGcov_la_LDFLAGS = @MODULE_SHARED_LDFLAGS@ -version-info 0:1:0 -no-undefined -avoid-version

libcbGcov_la_LIBADD = ../../../sdk/libcodeblocks.la $(WX_LIBS)

libcbGcov_la_SOURCES = cbGcov.cpp \
		cbGcovConfigPanel.cpp \
		cbGcovSummaryPanel.cpp \
		GcovProcess.cpp

noinst_HEADERS = cbGcov.h \
		cbGcovConfigPanel.h \
		cbGcovSummaryPanel.h \
		GcovProcess.h

resources_forZIP = manifest.xml \
		images/16x16/greenNext.png \
		images/16x16/greenPrev.png \
		images/16x16/redNext.png \
		images/16x16/redPrev.png \
		images/20x20/greenNext.png \
		images/20x20/greenPrev.png \
		images/20x20/redNext.png \
		images/20x20/redPrev.png \
		images/24x24/greenNext.png \
		images/24x24/greenPrev.png \
		images/24x24/redNext.png \
		images/24x24/redPrev.png \
		images/28x28/greenNext.png \
		images/28x28/greenPrev.png \
		images/28x28/redNext.png \
		images/28x28/redPrev.png \
		images/32x32/greenNext.png \
		images/32x32/greenPrev.png \
		images/32x32/redNext.png \
		images/32x32/redPrev.png \
		images/40x40/greenNext.png \
		images/40x40/greenPrev.png \
		images/40x40/redNext.png \
		images/40x40/redPrev.png \
		images/48x48/greenNext.png \
		images/48x48/greenPrev.png \
		images/48x48/redNext.png \
		images/48x48/redPrev.png \
		images/56x56/greenNext.png \
		images/56x56/greenPrev.png \
		images/56x56/redNext.png \
		images/56x56/redPrev.png \
		images/64x64/greenNext.png \
		images/64x64/greenPrev.png \
		images/64x64/redNext.png \
		images/64x64/redPrev.png \
		cb_gcov_toolbar.xrc

EXTRA_DIST = $(srcdir)/*.cbp \
		$(srcdir)/wxsmith/cbGcovConfigPanel.wxs \
		$(srcdir)/wxsmith/cbGcovSummaryPanel.wxs \
		$(resources_forZIP)

pkgdata_DATA = cbGcov.zip

CLEANFILES = $(pkgdata_DATA)

cbGcov.zip: $(resources_forZIP)
	PWD=`pwd` cd $(srcdir) && zip $(PWD)/cbGcov.zip $(resources_forZIP) > /dev/null
