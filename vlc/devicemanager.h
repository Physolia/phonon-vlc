/*****************************************************************************
 * libVLC backend for the Phonon library                                     *
 *                                                                           *
 * Copyright (C) 2009-2010 vlc-phonon AUTHORS                                *
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

#ifndef Phonon_VLC_DEVICEMANAGER_H
#define Phonon_VLC_DEVICEMANAGER_H

#include <phonon/audiooutputinterface.h>

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace VLC {

class Backend;
class DeviceManager;
class AbstractRenderer;
class VideoWidget;

/** \brief Container for information about devices supported by libVLC
 *
 * It includes a (hopefully unique) device identifier, a name identifier, a
 * description, a hardware identifier (may be a platform dependent device name),
 * and other relevant info.
 */
class DeviceInfo
{
public:
    enum Capability {
        None            = 0x0000,
        AudioOutput     = 0x0001,
        AudioCapture    = 0x0002,
        VideoCapture    = 0x0004
    };
public:
    DeviceInfo(const QByteArray& name, const QString& description = "");

    int id;
    QByteArray name;
    QString description;
    DeviceAccessList accessList;
    quint16 capabilities;
};

/** \brief Keeps track of audio/video devices that libVLC supports
 *
 * This class maintains a device list for each of the following device categories:
 * \li audio output devices
 * \li audio capture devices
 * \li video capture devices
 *
 * Methods are provided to retrieve information about these devices.
 *
 * \see EffectManager
 */
class DeviceManager : public QObject
{
    Q_OBJECT

public:
    DeviceManager(Backend *parent);
    virtual ~DeviceManager();
    const QList<DeviceInfo> audioCaptureDevices() const;
    const QList<DeviceInfo> videoCaptureDevices() const;
    const QList<DeviceInfo> audioOutputDevices() const;
    int deviceId(const QByteArray &vlcId) const;
    QString deviceDescription( int i_id) const;

signals:
    void deviceAdded(int);
    void deviceRemoved(int);

public slots:
    void updateDeviceList();

private:
    bool canOpenDevice() const;
    void updateDeviceSublist(const QList<DeviceInfo> &newDevices, QList<Phonon::VLC::DeviceInfo> &deviceList);

private:
    Backend *m_backend;
    QList<DeviceInfo> m_audioCaptureDeviceList;
    QList<DeviceInfo> m_videoCaptureDeviceList;
    QList<DeviceInfo> m_audioOutputDeviceList;
};
}
} // namespace Phonon::VLC

QT_END_NAMESPACE

#endif // Phonon_VLC_DEVICEMANAGER_H
