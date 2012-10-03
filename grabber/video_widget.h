#pragma once

#include <QWidget>

class QLabel;
class QImage;

class VideoWidget : public QWidget
{
	Q_OBJECT
public:
	VideoWidget(QWidget* parent = 0);

	void setImage(const QImage & img);

private Q_SLOTS:
	void doSetImage(const QImage & img);

private:
	QLabel* lbl_;

};
