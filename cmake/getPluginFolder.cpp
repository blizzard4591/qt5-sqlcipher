#include <iostream>

#include <QCoreApplication>
#include <QStringList>

int main() {
	QStringList const libraryPaths = QCoreApplication::libraryPaths();
	if (libraryPaths.size() == 0) {
		std::cerr << "No library paths registered!" << std::endl;
		return -1;
	}
	
	std::cout << libraryPaths.at(0).toStdString();

	return 0;
}