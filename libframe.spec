%define _prefix /usr
%define _docdir %{_datadir}/doc

Name: libframe
Version: 8.30
Release: 1%{?dist}
Summary: LIGO/VIRGO frame library
License: ?
Group: LAL
Source: %{name}-%{version}.tar.gz
URL: http://lappweb.in2p3.fr/virgo/FrameL
Packager: Adam Mercer <adam.mercer@ligo.org>
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Obsoletes: frame
Prefix: %{_prefix}
%description
A Common Data Frame Format for Interferometric Gravitational Wave Detector
has been developed by VIRGO and LIGO.  The Frame Library is a software
dedicated to the frame manipulation including file input/output.

This package contains the shared-object library needed to run libframe
applications.


%package devel
Summary: Files and documentation needed for compiling libframe programs
Group: LAL
Requires: %{name} = %{version}
%description devel
A Common Data Frame Format for Interferometric Gravitational Wave Detector
has been developed by VIRGO and LIGO.  The Frame Library is a software
dedicated to the frame manipulation including file input/output.

This package contains the files needed for building libframe programs, as
well as the documentation for the library.


%package utils
Summary: Frame file manipulation utilities
Group: LAL
Requires: %{name} = %{version}
%description utils
A Common Data Frame Format for Interferometric Gravitational Wave Detector
has been developed by VIRGO and LIGO.  The Frame Library is a software
dedicated to the frame manipulation including file input/output.

This package contains some utilities for manipulating frame files.


%package matlab
Summary: Matlab bindings for libframe
Group: LAL
Requires: %{name} = %{version}
%description matlab
A Common Data Frame Format for Interferometric Gravitational Wave Detector
has been developed by VIRGO and LIGO.  The Frame Library is a software
dedicated to the frame manipulation including file input/output.

This package contains Matlab bindings for manipulating frame files from
within Matlab.


%prep
rm -Rf ${RPM_BUILD_DIR}/%{name}-%{version}
%setup -q

%build
%configure
%{__make}

%install
%make_install docdir=%{_docdir}/%{name}-%{version}
rm -rf $RPM_BUILD_ROOT%{_libdir}/*.la

%post
ldconfig

%postun
ldconfig

%clean
[ ${RPM_BUILD_ROOT} != "/" ] && rm -Rf ${RPM_BUILD_ROOT}
rm -Rf ${RPM_BUILD_DIR}/%{name}-%{version}

%files
%defattr(-,root,root)
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root)
%{_libdir}/*.a
%{_libdir}/*.so
%{_libdir}/pkgconfig/*
%{_includedir}/*
%{_docdir}/*

%files utils
%defattr(-,root,root)
%{_bindir}/*

%files matlab
%defattr(-,root,root)
%{_datadir}/libframe/src/matlab/*


# dates should be formatted using: 'date +"%a %b %d %Y"'
%changelog
* Tue Apr 12 2016 Adam Mercer <adam.mercer@ligo.org> 8.30-1
- Update packaging for O2
