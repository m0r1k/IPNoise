%define package_name    squid
%define package_path    %{ipnoise_packages_path}/ipnoise-%{package_name}

%define name            ipnoise-squid
%define version         3.5.27
%define release         1

Name:           %{name}
Summary:        IPNoise squid
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/IPNoise

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/ipnoise-buildroot
License:        Copyright Roman E. Chechnev

Autoreq:        0
Provides:       squid
Requires:       /bin/perl, /bin/sh, libcom_err.so.2, libcrypt.so.1, libcrypt.so.1(GLIBC_2.0), libc.so.6, libc.so.6(GLIBC_2.0), libc.so.6(GLIBC_2.1), libc.so.6(GLIBC_2.1.3), libc.so.6(GLIBC_2.2), libc.so.6(GLIBC_2.2.3), libc.so.6(GLIBC_2.3), libc.so.6(GLIBC_2.3.2), libc.so.6(GLIBC_2.3.4), libc.so.6(GLIBC_2.6), libc.so.6(GLIBC_2.7), libdb-5.3.so, libdl.so.2, libdl.so.2(GLIBC_2.0), libdl.so.2(GLIBC_2.1), libgcc_s.so.1, libgcc_s.so.1(GCC_3.0), libgcc_s.so.1(GLIBC_2.0), libgssapi_krb5.so.2, libgssapi_krb5.so.2(gssapi_krb5_2_MIT), libk5crypto.so.3, libkrb5.so.3, libkrb5.so.3(krb5_3_MIT), libm.so.6, libm.so.6(GLIBC_2.0), libnsl.so.1, libnsl.so.1(GLIBC_2.0), libpthread.so.0, libpthread.so.0(GLIBC_2.0), libpthread.so.0(GLIBC_2.1), libpthread.so.0(GLIBC_2.2), libpthread.so.0(GLIBC_2.3.2), libresolv.so.2, librt.so.1, librt.so.1(GLIBC_2.1), librt.so.1(GLIBC_2.2), libstdc++.so.6, libstdc++.so.6(CXXABI_1.3), libstdc++.so.6(GLIBCXX_3.4), libstdc++.so.6(GLIBCXX_3.4.11), libstdc++.so.6(GLIBCXX_3.4.15), libstdc++.so.6(GLIBCXX_3.4.9), perl(Data::Dumper), perl(FileHandle), perl(Getopt::Std), perl(IPC::Open2), perl(Net::POP3), perl(vars), rpmlib(CompressedFileNames) <= 3.0.4-1, rpmlib(PayloadFilesHavePrefix) <= 4.0-1, rtld(GNU_HASH), /usr/bin/perl

%description
IPNoise squid
TODO add description here

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
    /usr/sbin/squid -z
    systemctl daemon-reload
    systemctl enable  ipnoise-squid
    systemctl restart ipnoise-squid.service

%preun
    systemctl disable   ipnoise-squid
    systemctl stop      ipnoise-squid.service

%changelog
* Tue Oct 28 2014 - morik@
- New version 3.2.1-4
- ipv6 was enabled, looks like it was kernel driver's bug (not poll events after connect) 

* Tue Oct 28 2014 - morik@
- New version 3.2.1-3
- ipv6 was disabled, because any ipv4 dst address is 0.0.0.0 (looks like it's squid's bug)

