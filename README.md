Qt SQL driver plugin for SQLCipher
==================================

This is a [QSqlDriverPlugin](http://doc.qt.io/qt-6/qsqldriverplugin.html) for
[SQLCipher](https://www.zetetic.net/sqlcipher/open-source/). It is quite
simple - it uses Qt's own SQLite driver code but links against SQLCipher
instead of SQLite.

## Dependencies

- Qt 6 (with private header files) (for Qt5 support, look in the old qt5 branch!)
- SQLCipher
- CMake >= 3.10
- pkg-config

On a Debian-like platform, you need to install the Qt6 private-dev packages:
```
	apt install qt6-base-dev qt6-base-private-dev libsodium-dev
```

## Tested platforms

- Debian 13 Trixie

    - Qt 6.8.2
    - SQLCipher 4.6.1
    - Also requires ``qt6-base-private-dev`` for Qt's private headers.

- Windows 11

    - Qt 6.10.0
    - hacked together

## Deployment

Follow [Qt's plugin deployment guide](http://doc.qt.io/qt-6/deployment-plugins.html).
In short, put the plugin at ``sqldrivers/libqsqlcipher.so`` relative to your
executable.


## Static linking

You can also build the plugin statically by passing ``-DSTATIC=ON`` to CMake.
When you build your application which uses the static plugin, you'll need to
include the line ``Q_IMPORT_PLUGIN(QSQLCipherDriverPlugin);`` in one of your
source files and define ``QT_STATICPLUGIN`` at compile time. And link to the
static plugin, of course.

Note that setting ``-DSTATIC=ON`` only builds *this plugin* as a static library.
If you also want to link to static versions of Qt and/or SQLCipher, it's up to
you to make sure CMake finds static versions of those libraries.


## Tests

Some basic tests are included - run ``make test``. Note that while pretty much
any C++ compiler can build the actual plugin, the tests require support for
C++17. If you have an old compiler you can pass ``-DBUILD_TESTING=OFF`` to CMake
to skip building the tests.

