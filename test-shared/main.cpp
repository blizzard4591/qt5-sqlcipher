#include <QCoreApplication>
#include <QtSql>
#include <QTemporaryDir>
#include <iostream>
#include <cstdlib>

#define QSQLCIPHER_TEST_ASSERT(cond, message) if(!(cond)) { std::cerr << message << std::endl; exit(-1); }

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	for (const auto& driver : QSqlDatabase::drivers()) {
		std::cout << "Available QSqlDatabase driver: " << driver.toStdString() << std::endl;
	}

	QSQLCIPHER_TEST_ASSERT(QSqlDatabase::isDriverAvailable("QSQLITE"), "QSQLITE driver is not available!"); // from Qt
	QSQLCIPHER_TEST_ASSERT(QSqlDatabase::isDriverAvailable("QSQLCIPHER"), "QSQLCIPHER driver is not available!"); // from our plugin

	QTemporaryDir tmp;
	QSQLCIPHER_TEST_ASSERT(tmp.isValid(), "Could not get temp directory.");

	auto withDB = [&](const char *driver, auto fn) {
		QString path = QDir(tmp.path()).absoluteFilePath(QString(driver) + ".db");
		{
			QSqlDatabase db = QSqlDatabase::addDatabase(driver, "db");
			db.setDatabaseName(path);
			QSQLCIPHER_TEST_ASSERT(db.open(), "Could not open database!");
			fn(db);
		}
		QSqlDatabase::removeDatabase("db");
	};

	// QSQLITE
	{
		std::cout << "Running Task 1..." << std::endl;
		// Create a SQLite db
		withDB("QSQLITE", [](auto db){
			db.exec("create table foo (bar integer)");
			db.exec("insert into foo values (42)");
		});

		std::cout << "Running Task 2..." << std::endl;
		// Check that we can read from the SQLite db
		withDB("QSQLITE", [](auto db){
			QSqlQuery q = db.exec("select bar from foo");
			QSQLCIPHER_TEST_ASSERT(q.next(), "Expected a fetchable entry in Query object.");
			QSQLCIPHER_TEST_ASSERT(q.value(0).toInt() == 42, "Database returned invalid value for row!");
		});

		std::cout << "Running Task 3..." << std::endl;
		// Check that SQLite is not SQLCipher
		withDB("QSQLITE", [](auto db){
			QSqlQuery q = db.exec("select sqlcipher_export()");
			QString errmsg = q.lastError().databaseText();
			QSQLCIPHER_TEST_ASSERT(errmsg.startsWith("no such function"), "Database did not respond with the expected error message.");
		});
	}

	// QSQLCIPHER
	{
#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER )
		std::cout << "Hint: If you get an error in the following task like \"QSQLCIPHER driver not loaded\" then check if library dependencies (e.g. libeay32.dll) are in the same folder as the test executable." << std::endl;
#endif
		std::cout << "Running Task 4..." << std::endl;
		// Check that SQLCipher is not SQLite
		withDB("QSQLCIPHER", [](auto db){
			QSqlQuery q = db.exec("select sqlcipher_export()");
			QString errmsg = q.lastError().databaseText();
			QSQLCIPHER_TEST_ASSERT(errmsg.startsWith("wrong number of arguments"), "Database did not respond with the expected error message.");
		});

		std::cout << "Running Task 5..." << std::endl;
		// Create a SQLCipher db with a passphrase
		withDB("QSQLCIPHER", [](auto db){
			std::cout << "Running Task 5.1..." << std::endl;
			db.exec("PRAGMA key = 'foobar';");
			std::cout << "Running Task 5.2... (this might trigger a segfault)" << std::endl;
			db.exec("CREATE TABLE `foo` (`bar`	INTEGER);");
			std::cout << "Running Task 5.3..." << std::endl;
			db.exec("INSERT INTO `foo` VALUES (42);");
		});

		std::cout << "Running Task 6..." << std::endl;
		// Check that we can't read from the SQLCipher db without the passphrase
		withDB("QSQLCIPHER", [](auto db){
			QSqlQuery q = db.exec("select bar from foo");
			QSQLCIPHER_TEST_ASSERT(!q.next(), "Query returned a fetchable row, this should not be the case.");
		});

		std::cout << "Running Task 7..." << std::endl;
		// Check that we can read from the SQLCipher db with the passphrase
		withDB("QSQLCIPHER", [](auto db){
			db.exec("PRAGMA key = 'foobar';");
			QSqlQuery q = db.exec("select bar from foo");
			QSQLCIPHER_TEST_ASSERT(q.next(), "Expected a fetchable entry in Query object.");
			QSQLCIPHER_TEST_ASSERT(q.value(0).toInt() == 42, "Database returned invalid value for row!");
		});
		std::cout << "Success! All tests completed." << std::endl;
	}

	return 0;
}
