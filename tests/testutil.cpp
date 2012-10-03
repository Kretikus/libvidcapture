#include "testutil.h"

#include <QMetaObject>
#include <QMetaMethod>
#include <QTest>
#include <QTextStream>

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

static QSet<QString> privateMethods()
{
	QSet<QString> ret;
	ret << "destroyed(QObject*)"
		<< "destroyed()"
		<< "deleteLater()"
		<< "_q_reregisterTimers(void*)";
	return ret;
}

void TestRunner::showTests()
{
	QTextStream out(stdout);
	QSet<QString> pm = privateMethods();
	Q_FOREACH(QObject* obj, testObjects_) {
		out << obj->metaObject()->className() << endl;
		for(int i = 0; i < obj->metaObject()->methodCount(); ++i) {
			QMetaMethod m = obj->metaObject()->method(i);
			if(!pm.contains(m.signature()) ) {
				out << "\t" << m.signature() << endl;
			}
		}
	}
}
