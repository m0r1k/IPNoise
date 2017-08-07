%define packages_path   ipnoise/IPNoise/packages/
%define package_name    thttpd
%define package_path    %{packages_path}/ipnoise-%{package_name}

%define name            ipnoise-thttpd
%define version         2.0.6
%define release         6

Name:           %{name} 
Summary:        IPNoise thttpd
Version:        %{version} 
Release:        %{release} 
URL:            http://ipnoise.ru/

Group:          Applications/Multimedia 
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman. E. Chechnev

%description
IPNoise thttpd
TODO add description here

%prep
# Look at http://rpm.org/max-rpm/s1-rpm-inside-macros.html
# for help with %setup
%setup -T -D -c -n ipnoise
%{git_clone}
cd ipnoise
%{git_checkout}
%{git_pull}

%build 
mkdir -p %buildroot
make -C %{package_path} lin32

%install
cd %{package_path}
make install DESTDIR=%buildroot
ipnoise-devel-rpms-find_files --buildroot "%buildroot" --out .files.list

%files
/etc/init.d/thttpd
/etc/thttpd.conf
/usr/sbin/thttpd
/var/run/thttpd/

%clean 
rm -rf $RPM_BUILD_ROOT 

%post
    /sbin/chkconfig --level 235 thttpd on   >/dev/null 2>&1     || echo -n
#    /sbin/service thttpd restart            >/dev/null 2>&1     || echo -n

%preun
#    /sbin/service thttpd stop               >/dev/null 2>&1     || echo -n
    /sbin/chkconfig --level 235 thttpd off  >/dev/null 2>&1     || echo -n

