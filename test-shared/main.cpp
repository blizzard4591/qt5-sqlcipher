#include <QCoreApplication>
#include <QDebug>
#include <QtSql>
#include <QTemporaryDir>
#include <iostream>

#ifndef QT_DEBUG
#error Must be built in debug mode!
#endif

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    for (const auto& driver : QSqlDatabase::drivers())
        qDebug() << driver;

    Q_ASSERT(QSqlDatabase::isDriverAvailable("QSQLITE")); // from Qt
    Q_ASSERT(QSqlDatabase::isDriverAvailable("QSQLCIPHER")); // from our plugin

    QTemporaryDir tmp;
    Q_ASSERT(tmp.isValid());

    auto withDB = [&](const char *driver, auto fn) {
        QString path = QDir(tmp.path()).absoluteFilePath(QString(driver) + ".db");
        {
            QSqlDatabase db = QSqlDatabase::addDatabase(driver, "db");
            db.setDatabaseName(path);
            Q_ASSERT(db.open());
            fn(db);
        }
        QSqlDatabase::removeDatabase("db");
    };

    // QSQLITE
    {
		std::cout << 1 << std::endl;
        // Create a SQLite db
        withDB("QSQLITE", [](auto db){
            db.exec("create table foo (bar integer)");
            db.exec("insert into foo values (42)");
        });

		std::cout << 2 << std::endl;
        // Check that we can read from the SQLite db
        withDB("QSQLITE", [](auto db){
            QSqlQuery q = db.exec("select bar from foo");
            Q_ASSERT(q.next());
            Q_ASSERT(q.value(0).toInt() == 42);
        });

		std::cout << 3 << std::endl;
        // Check that SQLite is not SQLCipher
        withDB("QSQLITE", [](auto db){
            QSqlQuery q = db.exec("select sqlcipher_export()");
            QString errmsg = q.lastError().databaseText();
            Q_ASSERT(errmsg.startsWith("no such function"));
        });
    }

    // QSQLCIPHER
    {
		std::cout << 4 << std::endl;
        // Check that SQLCipher is not SQLite
        withDB("QSQLCIPHER", [](auto db){
            QSqlQuery q = db.exec("select sqlcipher_export()");
            QString errmsg = q.lastError().databaseText();
            Q_ASSERT(errmsg.startsWith("wrong number of arguments"));
        });

		std::cout << 5 << std::endl;
        // Create a SQLCipher db with a passphrase
        withDB("QSQLCIPHER", [](auto db){
            db.exec("pragma key='foobar'");
            db.exec("create table foo (bar integer)");
            db.exec("insert into foo values (42)");
        });

		std::cout << 6 << std::endl;
        // Check that we can't read from the SQLCipher db without the passphrase
        withDB("QSQLCIPHER", [](auto db){
            QSqlQuery q = db.exec("select bar from foo");
            Q_ASSERT(!q.next());
        });

		std::cout << 7 << std::endl;
        // Check that we can read from the SQLCipher db with the passphrase
        withDB("QSQLCIPHER", [](auto db){
            db.exec("pragma key='foobar'");
            QSqlQuery q = db.exec("select bar from foo");
            Q_ASSERT(q.next());
            Q_ASSERT(q.value(0).toInt() == 42);
        });
    }

    return 0;
}
