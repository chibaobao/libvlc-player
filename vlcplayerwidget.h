#ifndef VLCPLAYERWIDGET_H
#define VLCPLAYERWIDGET_H
#include<stdint.h>
#include <QWidget>
#include <QImage>

#ifndef NO_VLC
#include <vlc.h>
#endif

#include "vlcplayerwidget_global.h"

typedef enum Player_state_t
{
    NothingSpecial=0,
    Opening,
    Buffering,
    Playing,
    Paused,
    Stopped,
    Ended,
    Error
} Player_state_t;

class I420Image
{
public:
	I420Image(int w, int h)
		: width(w)
		, height(h)
	{
		data = new uint8_t[w*h + w*h / 2];
	}
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }
	uint8_t *GetY() const { return data; }
	uint8_t *GetU()const { return data + width*height; }
	uint8_t *GetV()const { return data + width*height + width*height / 4; }
protected:
private:
	int width;
	int height;
	uint8_t * data;
};



class  VlcPlayerWidget : public QWidget //, public Player
{
    Q_OBJECT

public:
    explicit VlcPlayerWidget(QWidget *parent = 0);
    ~VlcPlayerWidget();

public:
    void setInput(QString input);
    void play();
    void pause();
    void stop();
    /**
     * @brief changePosition 用于视频跳转到几分之几位置
     * @param pos 要跳转的位置，position要在0-1之间，如0.1
     *            是跳转到十分之一位置
     */
    void changePosition(float position);
signals:
    void PlayerPaused();
    void PlayerStopped();
    void PlayerEndReached();
    void PlayerPlaying();

public:
    static void *lock_cb(void *opaque, void **planes);
    static void unlock_cb(void *opaque, void *picture, void *const *planes);
    static void display_cb(void *opaque, void *picture);
    static unsigned setup_cb(void **opaque, char *chroma,
                             unsigned *width, unsigned *height,
                             unsigned *pitches,
                             unsigned *lines);
    static void cleanup_cb(void *opaque);

protected:
    void paintEvent(QPaintEvent *event);
    static void stateChanged(const struct libvlc_event_t *event, void * data);

#ifndef NO_VLC
private:
    libvlc_instance_t* m_vlc;
    libvlc_media_player_t *m_vlcplayer;
    libvlc_event_manager_t *m_eventManager;
#endif

private:
    QString m_input;
	I420Image *m_Front;
	I420Image *m_Back;
	QImage *m_ImgShow;
};

#endif // VLCPLAYERWIDGET_H
