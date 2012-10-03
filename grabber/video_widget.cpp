#include "video_widget.h"

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
	lbl_->setPixmap(QPixmap::fromImage(img));
}
