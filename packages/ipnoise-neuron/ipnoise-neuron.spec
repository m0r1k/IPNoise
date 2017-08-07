%define packages_path   ipnoise/IPNoise/packages/
%define package_name    neuron
%define package_path    %{packages_path}/ipnoise-%{package_name}

%define name            ipnoise-neuron
%define version         0.0.1
%define release         0

Name:           %{name}
Summary:        IPNoise neuron
Version:        %{version}
Release:        %{release}
URL:            http://ipnoise.ru/

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman E. Chechnev

Requires:       mongodb-server

%description
IPNoise neuron
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
make -j4 -C %{package_path} lin32

%install
cd %{package_path}
make install DESTDIR=%buildroot
ipnoise-devel-rpms-find_files --buildroot "%buildroot" --out .files.list

%files
/lib/systemd/system/ipnoise-neuron.service
/usr/sbin/ipnoise-neuron

%clean
rm -rf $RPM_BUILD_ROOT

%post
    mkdir -p /etc/ipnoise/profile/;
    systemctl daemon-reload
    systemctl enable mongod
    systemctl enable ipnoise-neuron
    systemctl restart ipnoise-neuron.service

%preun
    systemctl disable   ipnoise-neuron
    systemctl stop      ipnoise-neuron.service

%postun
    rm -f /etc/systemd/system/multi-user.target.wants/ipnoise-neuron.service
    systemctl daemon-reload

