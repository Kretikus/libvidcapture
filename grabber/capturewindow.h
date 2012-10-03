#pragma once

#include <QWidget>

class QLabel;
class QImage;

class CaptureWindow : public QWidget
{
public:
	CaptureWindow(QWidget*parent = 0);

private:
	QLabel* lblCapture_;
};
