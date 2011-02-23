/*****************************************************************************
 * libVLC backend for the Phonon library                                     *
 *                                                                           *
 * Copyright (C) 2007-2008 Tanguy Krotoff <tkrotoff@gmail.com>               *
 * Copyright (C) 2008 Lukas Durfina <lukas.durfina@gmail.com>                *
 * Copyright (C) 2009 Fathi Boudra <fabo@kde.org>                            *
 * Copyright (C) 2009-2011 vlc-phonon AUTHORS                                *
 *                                                                           *
 * This program is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with this package; if not, write to the Free Software       *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "videowidget.h"

#include <QtCore/QtDebug>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>

#include <vlc/vlc.h>

#include "debug.h"
#ifndef PHONON_VLC_NO_EXPERIMENTAL
#include "experimental/avcapture.h"
#endif // PHONON_VLC_NO_EXPERIMENTAL
#include "mediaobject.h"

namespace Phonon
{
namespace VLC
{

VideoWidget::VideoWidget(QWidget *parent) :
    OverlayWidget(parent),
    SinkNode(),
    m_customRender(false),
    m_img(0),
    m_aspectRatio(Phonon::VideoWidget::AspectRatioAuto),
    m_scaleMode(Phonon::VideoWidget::FitInView),
    m_filterAdjustActivated(false),
    m_brightness(0.0),
    m_contrast(0.0),
    m_hue(0.0),
    m_saturation(0.0)
{
    setBackgroundColor(Qt::black);
}

VideoWidget::~VideoWidget()
{
    if (m_img) {
        delete m_img;
    }
}

void VideoWidget::connectToMediaObject(MediaObject *mediaObject)
{
    SinkNode::connectToMediaObject(mediaObject);

    connect(mediaObject, SIGNAL(videoWidgetSizeChanged(int, int)),
            SLOT(videoWidgetSizeChanged(int, int)));

    //  mediaObject->setVideoWidgetId(p_video_widget->winId());
    mediaObject->setVideoWidget(this);
}

#ifndef PHONON_VLC_NO_EXPERIMENTAL
void VideoWidget::connectToAvCapture(Experimental::AvCapture *avCapture)
{
    connectToMediaObject(avCapture->videoMediaObject());
}

void VideoWidget::disconnectFromAvCapture(Experimental::AvCapture *avCapture)
{
    disconnectFromMediaObject(avCapture->videoMediaObject());
}
#endif // PHONON_VLC_NO_EXPERIMENTAL

Phonon::VideoWidget::AspectRatio VideoWidget::aspectRatio() const
{
    return m_aspectRatio;
}

void VideoWidget::setAspectRatio(Phonon::VideoWidget::AspectRatio aspect)
{
    DEBUG_BLOCK;
    if (!m_player) {
        return;
    }

    m_aspectRatio = aspect;

    switch (m_aspectRatio) {
    case Phonon::VideoWidget::AspectRatioAuto:
        libvlc_video_set_aspect_ratio(m_player, 0);
        break;
    case Phonon::VideoWidget::AspectRatioWidget:
#ifdef __GNUC__
#warning might need work
#endif
        libvlc_video_set_aspect_ratio(m_player, 0);
        break;
    case Phonon::VideoWidget::AspectRatio4_3:
        libvlc_video_set_aspect_ratio(m_player, "4:3");
        break;
    case Phonon::VideoWidget::AspectRatio16_9:
        libvlc_video_set_aspect_ratio(m_player, "16:9");
        break;
    default:
        qCritical() << __FUNCTION__ << "error: unsupported AspectRatio:" << m_aspectRatio;
    }
}

Phonon::VideoWidget::ScaleMode VideoWidget::scaleMode() const
{
    return m_scaleMode;
}

void VideoWidget::setScaleMode(Phonon::VideoWidget::ScaleMode scale)
{
#ifdef __GNUC__
#warning OMG WTF
#endif
    m_scaleMode = scale;
    switch (m_scaleMode) {
    case Phonon::VideoWidget::FitInView: // The video will be fitted to fill the view keeping aspect ratio
        break;
    case Phonon::VideoWidget::ScaleAndCrop: // The video is scaled
        break;
    default:
        qWarning() << __FUNCTION__ << "unknow Phonon::VideoWidget::ScaleMode:" << m_scaleMode;
    }
}

qreal VideoWidget::brightness() const
{
    return m_brightness;
}

void VideoWidget::setBrightness(qreal brightness)
{
    DEBUG_BLOCK;
    if (!m_player) {
        return;
    }
    enableFilterAdjust();

    // VLC operates within a 0.0 to 2.0 range for brightness.
    m_brightness = brightness;
    debug() << "brightness: " << phononRangeToVlcRange(m_brightness);
    libvlc_video_set_adjust_float(m_player,
                                  libvlc_adjust_Brightness,
                                  phononRangeToVlcRange(m_brightness));
}

qreal VideoWidget::contrast() const
{
    return m_contrast;
}

void VideoWidget::setContrast(qreal contrast)
{
    DEBUG_BLOCK;
    if (!m_player) {
        return;
    }
    enableFilterAdjust();

    // VLC operates within a 0.0 to 2.0 range for contrast.
    m_contrast = contrast;
    libvlc_video_set_adjust_float(m_player,
                                  libvlc_adjust_Contrast,
                                  phononRangeToVlcRange(m_contrast));
}

qreal VideoWidget::hue() const
{
    return m_hue;
}

void VideoWidget::setHue(qreal hue)
{
    DEBUG_BLOCK;
    if (!m_player) {
        return;
    }
    enableFilterAdjust();

    // VLC operates within a 0 to 360 range for hue.
    m_hue = hue;
    libvlc_video_set_adjust_int(m_player,
                                libvlc_adjust_Hue,
                                static_cast<int>(phononRangeToVlcRange(m_hue, 360.0, false)));
}

qreal VideoWidget::saturation() const
{
    return m_saturation;
}

void VideoWidget::setSaturation(qreal saturation)
{
    DEBUG_BLOCK;
    if (!m_player) {
        return;
    }
    enableFilterAdjust();

    // VLC operates within a 0.0 to 3.0 range for saturation.
    m_saturation = saturation;
    libvlc_video_set_adjust_float(m_player,
                                  libvlc_adjust_Saturation,
                                  phononRangeToVlcRange(m_saturation, 3.0));
}

void VideoWidget::useCustomRender()
{
    m_customRender = true;
    QSize size = sizeHint();
    int width = size.width();
    int height = size.height();

    if (m_img) {
        delete m_img;
    }
    m_img = new QImage(size, QImage::Format_RGB32);
    libvlc_video_set_format(m_player, "RV32", width, height, width * 4);
    libvlc_video_set_callbacks(m_player, lock, unlock, 0, this);
}

void *VideoWidget::lock(void *data, void **bufRet)
{
    VideoWidget *cw = (VideoWidget *)data;
    cw->m_locker.lock();
    *bufRet = cw->m_img->bits();
    return NULL; // Picture identifier, not needed here.
}

void VideoWidget::unlock(void *data, void *id, void *const *pixels)
{
    Q_UNUSED(id);
    Q_UNUSED(pixels);

    VideoWidget *cw = (VideoWidget *)data;

    // Might be a good idea to cache these, but this should be insignificant overhead compared to the image conversion
//    const char *aspect = libvlc_video_get_aspect_ratio(cw->m_vlcPlayer);
//    delete aspect;

    cw->setNextFrame(QByteArray::fromRawData((const char *)cw->m_img->bits(),
                                     cw->m_img->byteCount()),
                                     cw->m_img->width(), cw->m_img->height());
    cw->m_locker.unlock();
}


QWidget *VideoWidget::widget()
{
    return this;
}

void VideoWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << "resizeEvent" << event->size();
}

void VideoWidget::setVideoSize(const QSize &size)
{
    m_videoSize = size;
    updateGeometry();
    update();
}

QSize VideoWidget::sizeHint() const
{
    if (!m_videoSize.isEmpty()) {
        return m_videoSize;
    }
    return QSize(640, 480);
}

void VideoWidget::setVisible(bool visible)
{
    if (window() && window()->testAttribute(Qt::WA_DontShowOnScreen)) {
        qDebug() << "Widget rendering forced";
        useCustomRender();
    }
    QWidget::setVisible(visible);
}

void VideoWidget::setNextFrame(const QByteArray &array, int width, int height)
{
    // TODO: Should preloading ever become available ... what do to here?

    m_frame = QImage();
    {
        m_frame = QImage((uchar *)array.constData(), width, height, QImage::Format_RGB32);
    }
    update();
}

void VideoWidget::videoWidgetSizeChanged(int width, int height)
{
    qDebug() << __FUNCTION__ << "video width" << width << "height:" << height;

    // It resizes dynamically the widget and the main window
    // Note: I didn't find another way

    QSize videoSize(width, height);
    videoSize.boundedTo(QApplication::desktop()->availableGeometry().size());

    hide();
    setVideoSize(videoSize);
#ifdef Q_OS_WIN
    QWidget *p_parent = qobject_cast<QWidget *>(this->parent());
    QSize previousSize = p_parent->minimumSize();
    p_parent->setMinimumSize(videoSize);
#endif
    show();
#ifdef Q_OS_WIN
    setMinimumSize(previousSize);
#endif

    if (m_img) {
        delete m_img;
    }
    m_img = new QImage(videoSize, QImage::Format_RGB32);
    libvlc_video_set_format(m_player, "RV32", width, height, width * 4);
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    if (m_customRender) {
        QPainter painter(this);
        // TODO: more sensible rect calculation.
        painter.drawImage(rect(), m_frame);
    } else {
        OverlayWidget::paintEvent(event);
    }
}

void VideoWidget::enableFilterAdjust(bool adjust)
{
    DEBUG_BLOCK;
    // Need to check for MO here, because we can get called before a VOut is actually
    // around in which case we just ignore this.
#ifdef __GNUC__
#warning TODO: if we need to ignore hue, brightness etc. sets we might as well set them pending
#endif
    if (!m_mediaObject || !m_mediaObject->hasVideo()) {
        debug() << "no mo or no video!!!";
        return;
    }
    if ((!m_filterAdjustActivated && adjust) ||
            (m_filterAdjustActivated && !adjust)) {
        debug() << "adjust: " << adjust;
        libvlc_video_set_adjust_int(m_player, libvlc_adjust_Enable, adjust);
        m_filterAdjustActivated = adjust;
    }
}

float VideoWidget::phononRangeToVlcRange(qreal phononValue, float upperBoundary,
                                         bool shift)
{
    DEBUG_BLOCK;
    // VLC operates on different ranges than Phonon. Phonon always uses a range of
    // -1:1 with 0 as the default value.
    // It is therefore necessary to convert between the two schemes using sophisticated magic.
    // First an incoming range is locked between the valid range of Phonon, then
    // everything is shifted into a positive value range.
    // Finally this range gets mapped onto a target range between 0 and the
    // provided upper boundary.
    float value = static_cast<float>(phononValue);
    int max = 1.0;

    // Ensure valid range
    if (value < -1.0) {
        value = -1.0;
    } else if (value > 1.0) {
        value = 1.0;
    }

    if (shift) {
        value += 1;
        max += 1;
    } else {
        if (value < 0) {
            value = 0;
        }
    }

    debug() << "incoming: " << phononValue;
    debug() << "shifted: " << value;
    debug() << "upperBoundary: " << upperBoundary;

    // value now holds a float between 0.0 and max.
    // If our upper boundary was 2 then we are done. If not we will have to
    // convert a range of 0-2 to 0-upperBoundary. This is achieved by calculating
    // the percentual value of value within the 0-2 range and then map this
    // to a value within the target range.

    float percentValue = (100.0 * value) / max;
    float ret = (upperBoundary * percentValue) / 100.0;

    debug() << "precent: " << percentValue;
    debug() << "ret: " << ret;

    return ret;
}

}
} // Namespace Phonon::VLC
