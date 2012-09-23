#pragma once

#include <QObject>
#include <QVector>

class TestRunner {
public:
	static TestRunner* instance();

	int registerClass(QObject*obj);
	void runTests(int argc, char* argv[]);

private:
	QVector<QObject*> testObjects_;
};

#define REGISTER_TEST(ClassName) \
	static const int ClassName##__INIT__ = \
			TestRunner::instance()->registerClass(new ClassName);

#define RUN_TESTS(argc, argv) \
{ \
	TestRunner::instance()->runTests(argc, argv); \
}
