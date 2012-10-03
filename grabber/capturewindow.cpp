#include "capturewindow.h"

#include <QLabel>
#include <QVBoxLayout>

CaptureWindow::CaptureWindow(QWidget *parent)
: QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	lblCapture_ = new QLabel(this);

	layout->addWidget(lblCapture_);
}
