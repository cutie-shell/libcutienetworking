#pragma once
#include <QDebug>
#include <QtQuick>
#include <QtQml/qqml.h>
#include <QtQml/QQmlExtensionPlugin>
#include <QDBusObjectPath>
#include "wifiaccesspoint_p.h"
#include "cutienetworkconnection_p.h"

class WifiSettingsPrivate : public QObject {
	Q_OBJECT
	Q_DECLARE_PUBLIC(WifiSettings)

    public:
	WifiSettingsPrivate(WifiSettings *q);
	~WifiSettingsPrivate();

    protected slots:
	void onNewConnection(QDBusObjectPath path);
	void onConnectionRemoved(QDBusObjectPath path);
	void onDeviceAdded(QDBusObjectPath path);
	void onDeviceRemoved(QDBusObjectPath path);
	void onAccessPointAdded(QDBusObjectPath path);
	void onAccessPointRemoved(QDBusObjectPath path);
	void onPropertiesChanged(QString iface, QMap<QString, QVariant> updated,
				 QStringList invalidated);
	void onDevicePropertiesChanged(QString iface,
				       QMap<QString, QVariant> updated,
				       QStringList invalidated);

    protected:
	QMap<QDBusObjectPath, WifiAccessPoint *> m_accessPoints;
	QDBusObjectPath m_activeAccessPoint;
	QDBusObjectPath m_wlanPath;
	QMap<QDBusObjectPath, CutieNetworkConnection *> m_savedConnections;
	bool m_wirelessEnabled;

    private:
	WifiSettings *q_ptr;
};