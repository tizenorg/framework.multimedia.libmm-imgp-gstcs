#sbs-git:slp/pkgs/l/libmm-imgp-gstcs libmm-imgp-gstcs 0.1 62b62e6d483557fc5750d1b4986e9a98323f1194
Name:       libmm-imgp-gstcs
Summary:    Multimedia Framework Utility Library
Version:    0.14
Release:    20
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post):  /sbin/ldconfig
Requires(postun):  /sbin/ldconfig
BuildRequires:  pkgconfig(mm-common)
BuildRequires:  pkgconfig(mm-log)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gmodule-2.0)
BuildRequires:  pkgconfig(gstreamer-app-1.0)
BuildRequires:  pkgconfig(gstreamer-video-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-base-1.0)
BuildRequires:  pkgconfig(gstreamer-pbutils-1.0)

BuildRoot:  %{_tmppath}/%{name}-%{version}-build

%description
Multimedia Framework Utility Library.

%package devel
Summary:    Multimedia Framework Utility Library (DEV)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Multimedia Framework Utility Library (DEV).

%prep
%setup -q

%build
./autogen.sh

export CFLAGS+=" -Wextra -Wno-array-bounds"
export CFLAGS+=" -Wno-ignored-qualifiers -Wno-unused-parameter -Wshadow"
export CFLAGS+=" -Wwrite-strings -Wswitch-default"
CFLAGS="$CFLAGS -DEXPORT_API=\"__attribute__((visibility(\\\"default\\\")))\" -D_MM_PROJECT_FLOATER" \
LDFLAGS+="-Wl,--rpath=%{_prefix}/lib -Wl,--hash-style=both -Wl,--as-needed" \
./configure --prefix=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2.0 %{buildroot}/usr/share/license/%{name}

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
/usr/share/license/%{name}
%manifest libmm-imgp-gstcs.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so*
