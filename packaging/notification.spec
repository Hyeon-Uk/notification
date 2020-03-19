%bcond_with wayland
Name:       notification
Summary:    Notification library
Version:    0.5.60
Release:    1
Group:      Applications/Core Applications
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(capi-appfw-app-control)
BuildRequires: pkgconfig(capi-appfw-app-common)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(iniparser)
BuildRequires: pkgconfig(security-manager)
BuildRequires: pkgconfig(libsmack)
BuildRequires: pkgconfig(gmock)
BuildRequires: pkgconfig(uuid)

BuildRequires: cmake
Requires(post): /sbin/ldconfig
Requires(post): %{TZ_SYS_BIN}/sqlite3
Requires(postun): /sbin/ldconfig

%define upgrade_script_path /usr/share/upgrade/scripts

%description
Client/Server library for sending notifications.

%if 0%{?gcov:1}
%package gcov
Summary:  Notification API(gcov)
Group:    Applications/Core Applications

%description gcov
gcov objects of a notification library
%endif

%prep
%setup -q

%package devel
Summary:    Notification library (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Development files needed to build software that needs to system a system notification.

%build
%if 0%{?gcov:1}
export CFLAGS+=" -fprofile-arcs -ftest-coverage"
export CXXFLAGS+=" -fprofile-arcs -ftest-coverage"
export FFLAGS+=" -fprofile-arcs -ftest-coverage"
export LDFLAGS+=" -lgcov"
%endif

MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . \
	-DCMAKE_INSTALL_PREFIX=%{_prefix} \
	-DMAJORVER=${MAJORVER} -DFULLVER=%{version}
%__make %{?jobs:-j%jobs}

%if 0%{?gcov:1}
mkdir -p gcov-obj
find . -name '*.gcno' -exec cp '{}' gcov-obj ';'
%endif

%check
ctest --output-on-failure %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

%if 0%{?gcov:1}
mkdir -p %{buildroot}%{_datadir}/gcov/obj
install -m 0644 gcov-obj/* %{buildroot}%{_datadir}/gcov/obj
%endif

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
%{_includedir}/notification/notification_type_internal.h
%{_libdir}/pkgconfig/notification.pc
%{_libdir}/libnotification.so



#################################################
# notification-ex
#################################################
%package -n %{name}-ex
Summary: Library for developing the notification
Group: Applications/Application Utility
License: Apache-2.0

%description -n %{name}-ex
Provide APIs to develop the notification.

%package -n %{name}-ex-devel
Summary: Notification development library (dev)
Group: Development/Libraries
Requires: %{name}-ex

%description -n %{name}-ex-devel
Header & package configuration files to support development of the notification.

%post -n %{name}-ex
/sbin/ldconfig

%postun -n %{name}-ex -p /sbin/ldconfig

%files -n %{name}-ex
%manifest %{name}-ex.manifest
%attr(0644,root,root) %{_libdir}/libnotification-ex.so.*
%license LICENSE

%files -n %{name}-ex-devel
%{_includedir}/notification-ex/*
%{_libdir}/pkgconfig/notification-ex.pc
%attr(0644,root,root) %{_libdir}/libnotification-ex.so


#################################################
# notification-ex_unittests
#################################################
%package -n notification-ex_unittests
Summary:    GTest for notification-ex
Group:      Development/Libraries

%description -n notification-ex_unittests
GTest for notification-ex

%files -n notification-ex_unittests
%{_bindir}/notification-ex_unittests

%if 0%{?gcov:1}
%files gcov
%{_datadir}/gcov/obj/*
%endif
