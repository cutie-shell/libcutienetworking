#include "wifiaccesspoint_p.h"

WifiAccessPoint::WifiAccessPoint(QObject *parent)
	: QObject(parent)
	, d_ptr(new WifiAccessPointPrivate(this))
{
}

WifiAccessPoint::~WifiAccessPoint()
{
	Q_D(WifiAccessPoint);
	delete d;
}

void WifiAccessPoint::setPath(QString path)
{
	Q_D(WifiAccessPoint);
	d->m_path = path;
	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager", path,
		"org.freedesktop.DBus.Properties", "PropertiesChanged", this,
		SLOT(onPropertiesChanged(QString, QMap<QString, QVariant>,
					 QStringList)));
	emit pathChanged(path);
	QDBusReply<QMap<QString, QVariant> > apReply =
		QDBusInterface("org.freedesktop.NetworkManager", path,
			       "org.freedesktop.DBus.Properties",
			       QDBusConnection::systemBus())
			.call("GetAll",
			      "org.freedesktop.NetworkManager.AccessPoint");
	if (apReply.isValid())
		d->m_data = apReply.value();
	else
		d->m_data = QMap<QString, QVariant>();
	emit dataChanged(d->m_data);
}

QString WifiAccessPoint::path()
{
	Q_D(WifiAccessPoint);
	return d->m_path;
}

QMap<QString, QVariant> WifiAccessPoint::data()
{
	Q_D(WifiAccessPoint);
	return d->m_data;
}

WifiAccessPointPrivate::WifiAccessPointPrivate(WifiAccessPoint *q)
	: q_ptr(q)
{
}

WifiAccessPointPrivate::~WifiAccessPointPrivate()
{
}

void WifiAccessPointPrivate::onPropertiesChanged(
	QString iface, QMap<QString, QVariant> updated, QStringList invalidated)
{
	Q_UNUSED(invalidated);
	Q_Q(WifiAccessPoint);
	if (iface == "org.freedesktop.NetworkManager.AccessPoint") {
		m_data.insert(updated);
		emit q->dataChanged(m_data);
	};
}