#include "testutil.h"

#include <QTest>

TestRunner* TestRunner::instance()
{
	static TestRunner* t = new TestRunner;
	return  t;
}

int TestRunner::registerClass(QObject* obj)
{
	if(obj) testObjects_.push_back(obj);
	return 0;
}

void TestRunner::runTests(int argc, char* argv[])
{
	Q_FOREACH(QObject* obj, testObjects_) {
		QTest::qExec(obj, argc, argv);
	}
}
