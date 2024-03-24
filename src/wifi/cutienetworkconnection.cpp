#include "cutienetworkconnection_p.h"

CutieNetworkConnection::CutieNetworkConnection(QObject *parent)
	: QObject(parent)
	, d_ptr(new CutieNetworkConnectionPrivate(this))
{
}

CutieNetworkConnection::~CutieNetworkConnection()
{
	Q_D(CutieNetworkConnection);
	delete d;
}

void CutieNetworkConnection::setPath(QString path)
{
	Q_D(CutieNetworkConnection);
	d->m_path = path;
	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager", path,
		"org.freedesktop.NetworkManager.Settings.Connection", "Updated",
		this, SLOT(onUpdated()));
	emit pathChanged(path);
	QDBusMessage reply =
		QDBusInterface(
			"org.freedesktop.NetworkManager", path,
			"org.freedesktop.NetworkManager.Settings.Connection",
			QDBusConnection::systemBus())
			.call("GetSettings");
	d->m_data = QMap<QString, QMap<QString, QVariant> >();
	const QDBusArgument &dbusArg =
		reply.arguments().at(0).value<QDBusArgument>();
	dbusArg >> d->m_data;
	emit dataChanged(data());
}

QString CutieNetworkConnection::path()
{
	Q_D(CutieNetworkConnection);
	return d->m_path;
}

QMap<QString, QVariant> CutieNetworkConnection::data()
{
	Q_D(CutieNetworkConnection);
	QMap<QString, QVariant> map;
	foreach(QString k, d->m_data.keys()) {
		map.insert(k, QVariant(d->m_data.value(k)));
	}
	return map;
}

void CutieNetworkConnection::deleteConnection()
{
	Q_D(CutieNetworkConnection);
	QDBusInterface("org.freedesktop.NetworkManager", d->m_path,
		       "org.freedesktop.NetworkManager.Settings.Connection",
		       QDBusConnection::systemBus())
		.call("Delete");
}

CutieNetworkConnectionPrivate::CutieNetworkConnectionPrivate(
	CutieNetworkConnection *q)
	: q_ptr(q)
{
}

CutieNetworkConnectionPrivate::~CutieNetworkConnectionPrivate()
{
}

void CutieNetworkConnectionPrivate::onUpdated()
{
	Q_Q(CutieNetworkConnection);
	QDBusMessage reply =
		QDBusInterface(
			"org.freedesktop.NetworkManager", m_path,
			"org.freedesktop.NetworkManager.Settings.Connection",
			QDBusConnection::systemBus())
			.call("GetSettings");
	m_data = QMap<QString, QMap<QString, QVariant> >();
	const QDBusArgument &dbusArg =
		reply.arguments().at(0).value<QDBusArgument>();
	dbusArg >> m_data;
	emit q->dataChanged(q->data());
}