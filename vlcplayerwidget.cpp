﻿#include "vlcplayerwidget.h"
#include <QPainter>
#include <QDebug>
extern "C"
{
#include "yuv2rgb.h"
}

#include <assert.h>



VlcPlayerWidget::VlcPlayerWidget(QWidget *parent) :
    QWidget(parent),
    m_vlcplayer(NULL),
    m_vlc(NULL),
    m_Front(NULL),
    m_Back(NULL)
{
    this->setGeometry(0, 0, 300, 100);
    QPalette pal(this->palette());

    //设置背景黑色
    pal.setColor(QPalette::Background, Qt::black);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    m_vlc = libvlc_new(0,0);

    m_vlcplayer = libvlc_media_player_new(m_vlc);
    libvlc_video_set_callbacks(m_vlcplayer,lock_cb,unlock_cb,display_cb,this);
    libvlc_video_set_format_callbacks(m_vlcplayer,setup_cb,cleanup_cb);
    m_eventManager = libvlc_media_player_event_manager(m_vlcplayer);
    libvlc_event_attach(m_eventManager,libvlc_MediaPlayerPaused,VlcPlayerWidget::stateChanged,this);
    libvlc_event_attach(m_eventManager,libvlc_MediaPlayerStopped,VlcPlayerWidget::stateChanged,this);
    libvlc_event_attach(m_eventManager,libvlc_MediaPlayerEndReached,VlcPlayerWidget::stateChanged,this);
    libvlc_event_attach(m_eventManager,libvlc_MediaPlayerPlaying,VlcPlayerWidget::stateChanged,this);
    libvlc_event_attach(m_eventManager,libvlc_MediaPlayerOpening,VlcPlayerWidget::stateChanged,this);
    this->show();
}
void VlcPlayerWidget::stateChanged(const struct libvlc_event_t *event, void * data)
{
    int type = event->type;
    VlcPlayerWidget *p_this = (VlcPlayerWidget*)data;
    switch (type) {
    case libvlc_MediaPlayerPaused:
        emit p_this->PlayerPaused();
        qDebug()<<"libvlc_MediaPlayerPaused";
        break;
    case libvlc_MediaPlayerStopped:
        emit p_this->PlayerStopped();
        qDebug()<<"libvlc_MediaPlayerStopped";
        break;
    case libvlc_MediaPlayerEndReached:
        emit p_this->PlayerEndReached();
        qDebug()<<"libvlc_MediaPlayerEndReached";
        break;
    case libvlc_MediaPlayerPlaying:
        emit p_this->PlayerPlaying();
        qDebug()<<"libvlc_MediaPlayerPlaying";
        break;

    default:
        break;
    }
}

VlcPlayerWidget::~VlcPlayerWidget()
{
    stop();
    libvlc_release(m_vlc);
}

void VlcPlayerWidget::setInput(QString input)
{
    m_input = input;
}

void VlcPlayerWidget::play()
{
    stop();
    libvlc_media_t *pmedia = libvlc_media_new_location(m_vlc,m_input.toLocal8Bit().data());

    libvlc_media_add_option(pmedia, ":rtsp-tcp=true :network-caching=300");
    libvlc_media_player_set_media(m_vlcplayer,pmedia);
    libvlc_media_player_play(m_vlcplayer);

//    libvlc_media_release(pmedia);
}

void VlcPlayerWidget::pause()
{
    if(libvlc_media_player_can_pause(m_vlcplayer))
    {
        libvlc_media_player_pause(m_vlcplayer);
    }
}

void VlcPlayerWidget::stop()
{
    libvlc_media_player_stop(m_vlcplayer);
    if(m_Front)
    {
        delete m_Front;
        m_Front = NULL;
    }
    if(m_Back)
    {
        delete m_Back;
        m_Back = NULL;
    }

}
void VlcPlayerWidget::changePosition(float position)
{
    if(position <0)
    {
        position = 0;
    }
    else if(position >1)
    {
        position = 1;
    }
    libvlc_media_player_set_position(m_vlcplayer, position);
}
void *VlcPlayerWidget::lock_cb(void *opaque, void **planes)
{
    VlcPlayerWidget *pthis = static_cast<VlcPlayerWidget*>(opaque);

    planes[0] = pthis->m_Back->GetY();
	planes[1] = pthis->m_Back->GetU();
	planes[2] = pthis->m_Back->GetV();
    return pthis->m_Back;
}

void VlcPlayerWidget::unlock_cb(void *opaque, void *picture, void * const *planes)
{
    VlcPlayerWidget *pthis = static_cast<VlcPlayerWidget*>(opaque);

	I420Image* p = pthis->m_Front;
    pthis->m_Front = pthis->m_Back;
    pthis->m_Back = p;
}

void VlcPlayerWidget::display_cb(void *opaque, void *picture)
{
    VlcPlayerWidget *pthis = static_cast<VlcPlayerWidget*>(opaque);

    pthis->update();
}

unsigned VlcPlayerWidget::setup_cb(void **opaque, char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
    VlcPlayerWidget *pthis = static_cast<VlcPlayerWidget*>(*opaque);
    assert(pthis);

	pthis->m_Front = new I420Image(*width, *height);
	pthis->m_Back = new I420Image(*width, *height);

	pthis->m_ImgShow = new QImage(pthis->m_Front->GetWidth(), pthis->m_Front->GetHeight(), QImage::Format_RGB888);

    pitches[0]=*width;
    lines[0] = *height;

	pitches[1] = pitches[2] = *width / 2;
	lines[1] = lines[2] = *height / 2;
    return 1;
}

void VlcPlayerWidget::cleanup_cb(void *opaque)
{

}

void VlcPlayerWidget::paintEvent(QPaintEvent *event)
{
	if (m_Front)
	{
		int width = m_Front->GetWidth();
		int height = m_Front->GetHeight();
		yuv420_2_rgb888(m_ImgShow->bits(), m_Front->GetY(), m_Front->GetU(), m_Front->GetV(), width, height, width, width >> 1, width * 3, yuv2rgb565_table, 0);
		int x, y, w, h;
		float aspect = m_Front->GetWidth()*1.0f / m_Front->GetHeight();
		if ((this->width()*1.0f / this->height()) > aspect)
		{
			h = this->height();
			w = h*aspect;
		}
		else
		{
			w = this->width();
			h = w / aspect;
		}

		x = (this->width() - w) / 2;
		y = (this->height() - h) / 2;
		
		QPainter draw(this);
		QRect rect(x, y, w, h);
		draw.drawImage(rect, m_ImgShow->scaled(w,h,Qt::KeepAspectRatio));


	}

    QWidget::paintEvent(event);
}
