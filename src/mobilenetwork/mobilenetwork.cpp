#include "mobilenetwork_p.h"
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusReply>

MobileNetwork::MobileNetwork(QObject *parent)
	: QObject(parent)
	, d_ptr(new MobileNetworkPrivate(this))
{
	Q_D(MobileNetwork);
	qDBusRegisterMetaType<QMap<QString, QMap<QString, QVariant> > >();
	qDBusRegisterMetaType<OfonoServicePair>();
	qDBusRegisterMetaType<OfonoServiceList>();

	QDBusInterface networkManager("org.freedesktop.NetworkManager",
				      "/org/freedesktop/NetworkManager",
				      "org.freedesktop.NetworkManager",
				      QDBusConnection::systemBus());

	QDBusReply<QList<QDBusObjectPath> > devicesReply =
		networkManager.call("GetDevices");
	if (!devicesReply.isValid())
		return;
	QList<QDBusObjectPath> devices = devicesReply.value();

	d->m_path = QDBusObjectPath("/");
	foreach(QDBusObjectPath path, devices) {
		QDBusInterface deviceInterface(
			"org.freedesktop.NetworkManager", path.path(),
			"org.freedesktop.DBus.Properties",
			QDBusConnection::systemBus());
		QDBusReply<QDBusVariant> typeReply = deviceInterface.call(
			"Get", "org.freedesktop.NetworkManager.Device",
			"DeviceType");
		if (typeReply.isValid()) {
			uint type =
				qdbus_cast<uint>(typeReply.value().variant());
			if (type == 8) {
				d->m_path = path;
				break;
			}
		}
	}

	if (d->m_path.path() == "")
		return;

	QDBusInterface deviceInterface("org.freedesktop.NetworkManager",
				       d->m_path.path(),
				       "org.freedesktop.DBus.Properties",
				       QDBusConnection::systemBus());

	d->m_mobileDataEnabled = false;

	QDBusReply<QDBusVariant> actConReply = deviceInterface.call(
		"Get", "org.freedesktop.NetworkManager.Device",
		"ActiveConnection");
	if (!actConReply.isValid()) {
		qDebug() << actConReply.error();
	} else {
		QDBusInterface actConIface(
			"org.freedesktop.NetworkManager",
			qdbus_cast<QDBusObjectPath>(
				actConReply.value().variant())
				.path(),
			"org.freedesktop.DBus.Properties",
			QDBusConnection::systemBus());
		QDBusReply<QDBusVariant> conReply = actConIface.call(
			"Get",
			"org.freedesktop.NetworkManager.Connection.Active",
			"Connection");
		if (!conReply.isValid()) {
			qDebug() << conReply.error();
		} else {
			d->m_activeConnection = qdbus_cast<QDBusObjectPath>(
				conReply.value().variant());
			d->m_mobileDataEnabled = d->m_activeConnection.path() !=
						 "/";
			emit activeConnectionChanged(
				d->m_activeConnection.path());
			emit mobileDataEnabledChanged(d->m_mobileDataEnabled);
		}
	}

	QDBusReply<QDBusVariant> enabledReply =
		QDBusInterface("org.freedesktop.NetworkManager",
			       "/org/freedesktop/NetworkManager",
			       "org.freedesktop.DBus.Properties",
			       QDBusConnection::systemBus())
			.call("Get", "org.freedesktop.NetworkManager",
			      "WwanEnabled");
	d->m_cellularEnabled =
		enabledReply.isValid() ?
			qdbus_cast<bool>(enabledReply.value().variant()) :
			false;

	QDBusReply<QDBusVariant> interfaceReply = deviceInterface.call(
		"Get", "org.freedesktop.NetworkManager.Device", "Interface");
	if (!interfaceReply.isValid()) {
		qDebug() << interfaceReply.error();
	} else {
		d->m_deviceId =
			qdbus_cast<QString>(interfaceReply.value().variant());
	}

	d->scanAvailableConnections();

	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager", d->m_path.path(),
		"org.freedesktop.DBus.Properties", "PropertiesChanged", d,
		SLOT(onDevicePropertiesChanged(QString, QMap<QString, QVariant>,
					       QStringList)));

	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager",
		"/org/freedesktop/NetworkManager",
		"org.freedesktop.DBus.Properties", "PropertiesChanged", d,
		SLOT(onPropertiesChanged(QString, QMap<QString, QVariant>,
					 QStringList)));
}

MobileNetwork::~MobileNetwork()
{
	Q_D(MobileNetwork);
	delete d;
}

QObject *MobileNetwork::provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
	Q_UNUSED(scriptEngine);
	return new MobileNetwork(engine);
}

bool MobileNetwork::mobileDataEnabled()
{
	Q_D(MobileNetwork);
	return d->m_mobileDataEnabled;
}

bool MobileNetwork::cellularEnabled()
{
	Q_D(MobileNetwork);
	return d->m_cellularEnabled;
}

QVariantList MobileNetwork::availableConnections()
{
	Q_D(MobileNetwork);
	return d->m_availableConnections;
}

QString MobileNetwork::activeConnection()
{
	Q_D(MobileNetwork);
	return d->m_activeConnection.path();
}

void MobileNetwork::setMobileDataEnabled(bool mobileDataEnabled)
{
	Q_D(MobileNetwork);
	if (!mobileDataEnabled) {
		QDBusInterface("org.freedesktop.NetworkManager",
			       d->m_path.path(),
			       "org.freedesktop.NetworkManager.Device",
			       QDBusConnection::systemBus())
			.call("Disconnect");
		d->m_mobileDataEnabled = mobileDataEnabled;
		emit mobileDataEnabledChanged(mobileDataEnabled);
	} else {
		if (d->m_activeConnection.path() == "/")
			return;
		d->m_mobileDataEnabled = true;
		setActiveConnection(d->m_activeConnection.path());
		emit mobileDataEnabledChanged(d->m_mobileDataEnabled);
	}
}

void MobileNetwork::setCellularEnabled(bool cellularEnabled)
{
	QDBusMessage message = QDBusMessage::createMethodCall(
		"org.freedesktop.NetworkManager",
		"/org/freedesktop/NetworkManager",
		"org.freedesktop.DBus.Properties", "Set");
	QList<QVariant> arguments;
	arguments << "org.freedesktop.NetworkManager" << "WwanEnabled"
		  << QVariant::fromValue(QDBusVariant(cellularEnabled));
	message.setArguments(arguments);
	QDBusConnection::systemBus().call(message);
}

void MobileNetwork::setActiveConnection(QString path)
{
	Q_D(MobileNetwork);
	setMobileDataEnabled(false);

	QDBusReply<QDBusObjectPath> connsReply =
		QDBusInterface("org.freedesktop.NetworkManager",
			       "/org/freedesktop/NetworkManager",
			       "org.freedesktop.NetworkManager",
			       QDBusConnection::systemBus())
			.call("ActivateConnection", QDBusObjectPath(path),
			      d->m_path, QDBusObjectPath("/"));

	if (!connsReply.isValid()) {
		qDebug() << connsReply.error();
	} else {
		d->m_activeConnection = QDBusObjectPath(path);
		d->scanAvailableConnections();
		foreach(QVariant connection, d->m_availableConnections) {
			if (connection.toMap().value("path").toString() ==
			    path) {
				d->syncOfonoContext(connection.toMap()
							    .value("name")
							    .toString(),
						    connection.toMap()
							    .value("apn")
							    .toString());
			}
		}
		emit activeConnectionChanged(d->m_activeConnection.path());
	}
}

QString MobileNetwork::addConnection(QString con_name, QString apn)
{
	Q_D(MobileNetwork);
	bool wasMobileDataEnabled = d->m_mobileDataEnabled;
	setMobileDataEnabled(false);

	QMap<QString, QMap<QString, QVariant> > data;
	QMap<QString, QVariant> conn;
	conn.insert("id", con_name);
	data.insert("connection", conn);

	QMap<QString, QVariant> gsm;
	gsm.insert("auto-config", false);
	gsm.insert("apn", apn);
	gsm.insert("device-id", d->m_deviceId);
	data.insert("gsm", gsm);

	QDBusReply<QDBusObjectPath> connsReply =
		QDBusInterface("org.freedesktop.NetworkManager",
			       "/org/freedesktop/NetworkManager/Settings",
			       "org.freedesktop.NetworkManager.Settings",
			       QDBusConnection::systemBus())
			.call("AddConnection",
			      QVariant::fromValue<
				      QMap<QString, QMap<QString, QVariant> > >(
				      data));

	if (!connsReply.isValid()) {
		qDebug() << connsReply.error();
	} else {
		d->scanAvailableConnections();
		d->syncOfonoContext(con_name, apn);
		emit mobileDataEnabledChanged(d->m_mobileDataEnabled);
		setMobileDataEnabled(wasMobileDataEnabled);
		return connsReply.value().path();
	}
	return "/";
}

void MobileNetwork::addAndActivateConnection(QString con_name, QString apn)
{
	QString conPath = addConnection(con_name, apn);
	setActiveConnection(conPath);
}

void MobileNetwork::updateConnection(QString path, QString con_name,
				     QString apn)
{
	Q_D(MobileNetwork);
	bool wasMobileDataEnabled = d->m_mobileDataEnabled;
	setMobileDataEnabled(false);

	QMap<QString, QMap<QString, QVariant> > data;

	QMap<QString, QVariant> conn;
	conn.insert("id", con_name);
	data.insert("connection", conn);

	QMap<QString, QVariant> gsm;
	gsm.insert("auto-config", false);
	gsm.insert("apn", apn);
	gsm.insert("device-id", d->m_deviceId);
	data.insert("gsm", gsm);
	QDBusInterface("org.freedesktop.NetworkManager", path,
		       "org.freedesktop.NetworkManager.Settings.Connection",
		       QDBusConnection::systemBus())
		.call("Update",
		      QVariant::fromValue<
			      QMap<QString, QMap<QString, QVariant> > >(data));

	d->scanAvailableConnections();
	d->syncOfonoContext(con_name, apn);
	setMobileDataEnabled(wasMobileDataEnabled);
}

void MobileNetwork::deleteConnection(QString path)
{
	Q_D(MobileNetwork);
	bool wasMobileDataEnabled = d->m_mobileDataEnabled;
	setMobileDataEnabled(false);
	QDBusInterface("org.freedesktop.NetworkManager", path,
		       "org.freedesktop.NetworkManager.Settings.Connection",
		       QDBusConnection::systemBus())
		.call("Delete");

	if (d->m_activeConnection.path() == path) {
		d->m_activeConnection.setPath("/");
		activeConnectionChanged(d->m_activeConnection.path());
	} else {
		setMobileDataEnabled(wasMobileDataEnabled);
	}

	d->scanAvailableConnections();
}

MobileNetworkPrivate::MobileNetworkPrivate(MobileNetwork *q)
	: q_ptr(q)
{
}

MobileNetworkPrivate::~MobileNetworkPrivate()
{
}

void MobileNetworkPrivate::onDevicePropertiesChanged(
	QString iface, QMap<QString, QVariant> updated, QStringList invalidated)
{
	Q_Q(MobileNetwork);
	Q_UNUSED(invalidated);
	if (iface == "org.freedesktop.NetworkManager.Device") {
		if (updated.contains("ActiveConnection")) {
			QDBusInterface actConIface(
				"org.freedesktop.NetworkManager",
				qdbus_cast<QDBusObjectPath>(
					updated.value("ActiveConnection"))
					.path(),
				"org.freedesktop.DBus.Properties",
				QDBusConnection::systemBus());
			QDBusReply<QDBusVariant> conReply = actConIface.call(
				"Get",
				"org.freedesktop.NetworkManager.Connection.Active",
				"Connection");
			if (!conReply.isValid()) {
				qDebug() << conReply.error();
				m_activeConnection = QDBusObjectPath("/");
			} else {
				m_activeConnection =
					qdbus_cast<QDBusObjectPath>(
						conReply.value().variant());
			}

			m_mobileDataEnabled = m_activeConnection.path() != "/";
			emit q->activeConnectionChanged(
				m_activeConnection.path());
			emit q->mobileDataEnabledChanged(m_mobileDataEnabled);
		}
	}
}

void MobileNetworkPrivate::onPropertiesChanged(QString iface,
					       QMap<QString, QVariant> updated,
					       QStringList invalidated)
{
	Q_Q(MobileNetwork);
	Q_UNUSED(invalidated);
	if (iface == "org.freedesktop.NetworkManager.Device") {
		if (updated.contains("WwanEnabled")) {
			m_cellularEnabled =
				qdbus_cast<bool>(updated.value("WwanEnabled"));
			emit q->cellularEnabledChanged(m_cellularEnabled);
		}
	}
}

QMap<QString, QMap<QString, QVariant> >
MobileNetworkPrivate::getConnectionSettings(QDBusObjectPath path)
{
	QDBusReply<QMap<QString, QMap<QString, QVariant> > > connsReply =
		QDBusInterface(
			"org.freedesktop.NetworkManager", path.path(),
			"org.freedesktop.NetworkManager.Settings.Connection",
			QDBusConnection::systemBus())
			.call("GetSettings");

	if (!connsReply.isValid()) {
		qDebug() << connsReply.error();
		return QMap<QString, QMap<QString, QVariant> >();
	} else {
		return connsReply;
	}
}

void MobileNetworkPrivate::scanAvailableConnections()
{
	Q_Q(MobileNetwork);
	QDBusInterface deviceInterface("org.freedesktop.NetworkManager",
				       m_path.path(),
				       "org.freedesktop.DBus.Properties",
				       QDBusConnection::systemBus());

	QDBusReply<QDBusVariant> availableCon = deviceInterface.call(
		"Get", "org.freedesktop.NetworkManager.Device",
		"AvailableConnections");

	if (!availableCon.isValid()) {
		qDebug() << availableCon.error();
	} else {
		QList<QDBusObjectPath> repList =
			qdbus_cast<QList<QDBusObjectPath> >(
				availableCon.value().variant());
		QVariantList conList;
		if (!repList.isEmpty()) {
			foreach(QDBusObjectPath path, repList) {
				if (path.path() == "/")
					continue;
				QVariantMap conMap;
				QMap<QString, QMap<QString, QVariant> >
					settings = getConnectionSettings(path);
				conMap.insert("path", path.path());
				conMap.insert("name",
					      settings.value("connection")
						      .value("id"));
				conMap.insert(
					"apn",
					settings.value("gsm").value("apn"));
				conList << conMap;
			}
			m_availableConnections = conList;
			emit q->availableConnectionsChanged();
		}
	}
}

void MobileNetworkPrivate::syncOfonoContext(QString name, QString apn)
{
	QDBusPendingReply<OfonoServiceList> contexts =
		QDBusInterface("org.ofono", "/ril_0",
			       "org.ofono.ConnectionManager",
			       QDBusConnection::systemBus())
			.call("GetContexts");
	contexts.waitForFinished();

	if (contexts.isValid()) {
		foreach(QPair pair, contexts.value()) {
			QString contextPath = pair.first.path();
			if (pair.second.value("Type").toString() != "internet")
				continue;

			if (pair.second.value("AccessPointName").toString() !=
				    apn ||
			    pair.second.value("Name").toString() != name) {
				setOfonoContextProperty(contextPath, "Active",
							false);
				setOfonoContextProperty(contextPath,
							"AccessPointName", apn);
				setOfonoContextProperty(contextPath, "Name",
							name);
				setOfonoContextProperty(contextPath, "Active",
							true);
			}
		}
	} else {
		qDebug() << contexts.error();
	}
}

void MobileNetworkPrivate::setOfonoContextProperty(QString path,
						   QString property,
						   QVariant value)
{
	QDBusInterface("org.ofono", path, "org.ofono.ConnectionContext",
		       QDBusConnection::systemBus())
		.call("SetProperty", property,
		      QVariant::fromValue(QDBusVariant(value)));
}