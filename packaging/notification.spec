%bcond_with wayland
Name:       notification
Summary:    Notification library
Version:    0.3.25
Release:    1
Group:      TBD
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(capi-appfw-app-control)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(iniparser)
BuildRequires: pkgconfig(security-manager)
BuildRequires: pkgconfig(libsmack)

BuildRequires: cmake
Requires(post): /sbin/ldconfig
Requires(post): %{TZ_SYS_BIN}/sqlite3
Requires(postun): /sbin/ldconfig

%define upgrade_script_path /usr/share/upgrade/scripts

%description
Client/Server library for sending notifications.

%prep
%setup -q

%package devel
Summary:    Notification library (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Development files needed to build software that needs to system a system notification.

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
export LDFLAGS+="-Wl,--rpath=%{_prefix}/lib -Wl,--as-needed"
LDFLAGS="$LDFLAGS"
%if %{with wayland}
%cmake . \
	-DCMAKE_INSTALL_PREFIX=%{_prefix} -DHAVE_WAYLAND=On \
	-DMAJORVER=${MAJORVER} -DFULLVER=%{version}
%else
%cmake . \
	-DCMAKE_INSTALL_PREFIX=%{_prefix} -DHAVE_X11=On \
	-DMAJORVER=${MAJORVER} -DFULLVER=%{version}
%endif
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{upgrade_script_path}
cp -f scripts/505.notification_upgrade.sh %{buildroot}%{upgrade_script_path}

%clean
rm -rf %{buildroot}

%post
/sbin/ldconfig

if [ ! -d %{TZ_SYS_DB} ]
then
	mkdir %{TZ_SYS_DB}
fi

%postun -p /sbin/ldconfig

%files
%manifest notification.manifest
%defattr(-,root,root,-)
%{_libdir}/libnotification.so.*
%license LICENSE
%attr(755,root,root) %{_sysconfdir}/gumd/useradd.d/11_notification-add.post
%{_bindir}/notification_init
%{upgrade_script_path}/505.notification_upgrade.sh
#%{_bindir}/notification-test-app

%files devel
%defattr(-,root,root,-)
%{_includedir}/notification/notification.h
%{_includedir}/notification/notification_db.h
%{_includedir}/notification/notification_internal.h
%{_includedir}/notification/notification_error.h
%{_includedir}/notification/notification_type.h
%{_includedir}/notification/notification_list.h
%{_includedir}/notification/notification_ongoing.h
%{_includedir}/notification/notification_ongoing_flag.h
%{_includedir}/notification/notification_text_domain.h
%{_includedir}/notification/notification_status.h
%{_includedir}/notification/notification_status_internal.h
%{_includedir}/notification/notification_setting.h
%{_includedir}/notification/notification_setting_internal.h
%{_includedir}/notification/notification_ipc.h
%{_includedir}/notification/notification_noti.h
%{_includedir}/notification/notification_setting_service.h
%{_includedir}/notification/notification_viewer.h
%{_includedir}/notification/notification_shared_file.h
%{_libdir}/pkgconfig/notification.pc
%{_libdir}/libnotification.so
