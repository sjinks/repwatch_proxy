# repwatch_proxy [![Build Status](https://travis-ci.org/sjinks/repwatch_proxy.png?branch=master)](https://travis-ci.org/sjinks/repwatch_proxy)

## Submodule Build Status

* [qt_eventdispatcher_libevent](https://github.com/sjinks/qt_eventdispatcher_libevent): [![Build Status](https://secure.travis-ci.org/sjinks/qt_eventdispatcher_libevent.png)](http://travis-ci.org/sjinks/qt_eventdispatcher_libevent)
* [qt_signalwatcher](https://github.com/sjinks/qt_signalwatcher): [![Build Status](https://secure.travis-ci.org/sjinks/qt_signalwatcher.png)](http://travis-ci.org/sjinks/qt_signalwatcher)
* [qt_socketconnector](https://github.com/sjinks/qt_socketconnector): [![Build Status](https://travis-ci.org/sjinks/qt_socketconnector.png?branch=master)](https://travis-ci.org/sjinks/qt_socketconnector)

## Install on CentOS

```bash
yum install qt4-devel libevent-devel gcc gcc-c++ git
git clone --quiet --branch=master git://github.com/sjinks/repwatch_proxy.git repwatch_proxy
cd repwatch_proxy
git submodule update --init --recursive
/usr/lib64/qt4/bin/qmake -recursive
make
```
