# repwatch_proxy [![Build Status](https://travis-ci.org/sjinks/repwatch_proxy.png?branch=master)](https://travis-ci.org/sjinks/repwatch_proxy)

## Submodule Build Status

* [qt_eventdispatcher_libevent](https://github.com/sjinks/qt_eventdispatcher_libevent): [![Build Status](https://secure.travis-ci.org/sjinks/qt_eventdispatcher_libevent.png)](http://travis-ci.org/sjinks/qt_eventdispatcher_libevent)
* [qt_signalwatcher](https://github.com/sjinks/qt_signalwatcher): [![Build Status](https://secure.travis-ci.org/sjinks/qt_signalwatcher.png)](http://travis-ci.org/sjinks/qt_signalwatcher)
* [qt_socketconnector](https://github.com/sjinks/qt_socketconnector): [![Build Status](https://travis-ci.org/sjinks/qt_socketconnector.png?branch=master)](https://travis-ci.org/sjinks/qt_socketconnector)

## Notes

* To be able to clone the repository please make sure that 9418/TCP port is open

## Build on CentOS

```bash
yum install qt4-devel libevent-devel gcc gcc-c++ git pam-devel tcp_wrappers
git clone --quiet --branch=master git://github.com/sjinks/repwatch_proxy.git repwatch_proxy
cd repwatch_proxy
git submodule update --init --recursive
/usr/lib64/qt4/bin/qmake -recursive
make
```

`pam-devel` and `tcp_wrappers` are optional.

If `pam-devel` is available, `repwatchproxy` will be built with [PAM](http://en.wikipedia.org/wiki/Pluggable_authentication_module) support.

If `tcp_wrappers` is available, `repwatchproxy` will be built with [TCP Wrapper](http://en.wikipedia.org/wiki/TCP_Wrapper) support.

PAM support can be optionally disabled with `qmake CONFIG+=nopam`

TCP Wrapper support can be optionally disabled with `qmake CONFIG+=nolibwrap`

## Build on Debian/Ubuntu

```bash
yum install libqt4-dev libevent-dev gcc g++ git libpam0g-dev libwrap0-dev
git clone --quiet --branch=master git://github.com/sjinks/repwatch_proxy.git repwatch_proxy
cd repwatch_proxy
git submodule update --init --recursive
qmake -recursive
make
```

`libpam0g-dev` and `libwrap0-dev` are optional.

If `libpam0g-dev` is available, `repwatchproxy` will be built with [PAM](http://en.wikipedia.org/wiki/Pluggable_authentication_module) support.

If `libwrap0-dev` is available, `repwatchproxy` will be built with [TCP Wrapper](http://en.wikipedia.org/wiki/TCP_Wrapper) support.

PAM support can be optionally disabled with `qmake CONFIG+=nopam`

TCP Wrapper support can be optionally disabled with `qmake CONFIG+=nolibwrap`
