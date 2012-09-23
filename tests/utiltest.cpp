#include <libvidcapture/util.h>
#include "testutil.h"

#include <QObject>
#include <QTest>

#include <string>

class UtilTest : public QObject { Q_OBJECT

private slots:

void stringConversionTest()
{
	std::wstring w = L"TEST String@!";
	std::string  s = "TEST String@!";

	// toStdString tests
	{
		std::string c = StringConversion::toStdString(w);
		QCOMPARE(c, s);
	}
	{
		std::string c = StringConversion::toStdString(L"");
		QCOMPARE(c, std::string());
	}

	// toStdWString test
	{
		std::wstring c = StringConversion::toStdWString(s);
		QCOMPARE(c, w);
	}
	{
		std::wstring c = StringConversion::toStdWString("");
		QCOMPARE(c, std::wstring());
	}

}

};

#include "utiltest.moc"

REGISTER_TEST(UtilTest)
