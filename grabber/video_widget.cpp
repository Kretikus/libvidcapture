#include "video_widget.h"

#include <QMetaObject>
#include <QLabel>
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>


VideoWidget::VideoWidget(QWidget* parent)
: QWidget(parent), lbl_(new QLabel(this))
{
	QVBoxLayout* l = new QVBoxLayout(this);
	l->addWidget(lbl_);
	setLayout(l);
}

void VideoWidget::setImage( const QImage & img )
{
	metaObject()->invokeMethod(this, "doSetImage", Qt::QueuedConnection, Q_ARG(QImage, img) );
}

void VideoWidget::doSetImage(const QImage &img)
{
	lbl_->setPixmap(QPixmap::fromImage(img));
}
