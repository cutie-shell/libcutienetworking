#pragma once
#include <QDebug>
#include <QtQuick>
#include <QtQml/qqml.h>
#include <QtQml/QQmlExtensionPlugin>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusInterface>
#include "../cutienetworking.h"

class WifiAccessPointPrivate : public QObject {
	Q_OBJECT
	Q_DECLARE_PUBLIC(WifiAccessPoint)

    public:
	WifiAccessPointPrivate(WifiAccessPoint *q);
	~WifiAccessPointPrivate();

    protected slots:
	void onPropertiesChanged(QString iface, QMap<QString, QVariant> updated,
				 QStringList invalidated);

    protected:
	QString m_path;
	QMap<QString, QVariant> m_data;

    private:
	WifiAccessPoint *q_ptr;
};