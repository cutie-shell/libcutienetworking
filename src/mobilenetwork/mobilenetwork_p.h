#pragma once
#include <QDebug>
#include <QtQuick>
#include <QtQml/qqml.h>
#include <QtQml/QQmlExtensionPlugin>
#include "../cutienetworking.h"

class MobileNetworkPrivate : public QObject {
	Q_OBJECT
	Q_DECLARE_PUBLIC(MobileNetwork)

    public:
	MobileNetworkPrivate(MobileNetwork *q);
	~MobileNetworkPrivate();

    protected slots:
	void onDevicePropertiesChanged(QString iface,
				       QMap<QString, QVariant> updated,
				       QStringList invalidated);
	void onPropertiesChanged(QString iface, QMap<QString, QVariant> updated,
				 QStringList invalidated);

    protected:
	QDBusObjectPath m_path;
	QString m_deviceId;
	QDBusObjectPath m_activeConnection;
	bool m_mobileDataEnabled;
	bool m_cellularEnabled;

	QVariantList m_availableConnections;
	QMap<QString, QMap<QString, QVariant> >
	getConnectionSettings(QDBusObjectPath path);
	void scanAvailableConnections();
	void syncOfonoContext(QString name, QString apn);
	void setOfonoContextProperty(QString path, QString property,
				     QVariant value);

    private:
	MobileNetwork *q_ptr;
};
