%define packages_path   ipnoise/IPNoise/packages/
%define package_name    nginx
%define package_path    %{packages_path}/ipnoise-%{package_name}

%define name            ipnoise-nginx
%define version         1.5.8
%define release         4

Name:           %{name}
Summary:        IPNoise nginx
Version:        %{version}
Release:        %{release}
URL:            http://ipnoise.ru/

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman. E. Chechnev

%description
IPNoise nginx support
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

%files -f %{package_path}/.files.list

%clean
rm -rf $RPM_BUILD_ROOT

%post
    adduser -r nginx -d /etc/nginx/ -s /sbin/nologin >/dev/null 2>&1
    mkdir -p /var/log/nginx/
    chown -R nginx:nginx /var/log/nginx/
    systemctl daemon-reload
    systemctl enable  ipnoise-nginx
    systemctl restart ipnoise-nginx.service

%preun
    systemctl disable   ipnoise-nginx
    systemctl stop      ipnoise-nginx.service

%changelog
* Mon Oct 20 2014 - morik@
- New version 1.5.8-4
- Enable IPv6 support 

