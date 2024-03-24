#include "wifisettings_p.h"
#include <QDBusMetaType>

WifiSettings::WifiSettings(QObject *parent)
	: QObject(parent)
	, d_ptr(new WifiSettingsPrivate(this))
{
	Q_D(WifiSettings);
	qDBusRegisterMetaType<QMap<QString, QMap<QString, QVariant> > >();

	QDBusInterface networkManager("org.freedesktop.NetworkManager",
				      "/org/freedesktop/NetworkManager",
				      "org.freedesktop.NetworkManager",
				      QDBusConnection::systemBus());

	QDBusInterface("org.freedesktop.NetworkManager",
		       "/org/freedesktop/NetworkManager/AgentManager",
		       "org.freedesktop.NetworkManager.AgentManager",
		       QDBusConnection::systemBus())
		.call("Register", "org.cutie_shell");

	QDBusReply<QDBusVariant> enabledReply =
		QDBusInterface("org.freedesktop.NetworkManager",
			       "/org/freedesktop/NetworkManager",
			       "org.freedesktop.DBus.Properties",
			       QDBusConnection::systemBus())
			.call("Get", "org.freedesktop.NetworkManager",
			      "WirelessEnabled");
	d->m_wirelessEnabled =
		enabledReply.isValid() ?
			qdbus_cast<bool>(enabledReply.value().variant()) :
			false;

	QDBusReply<QList<QDBusObjectPath> > devicesReply =
		networkManager.call("GetDevices");

	if (devicesReply.isValid()) {
		QList<QDBusObjectPath> devices = devicesReply.value();
		foreach(QDBusObjectPath path, devices)
			d->onDeviceAdded(path);
	}

	QDBusReply<QList<QDBusObjectPath> > connsReply =
		QDBusInterface("org.freedesktop.NetworkManager",
			       "/org/freedesktop/NetworkManager/Settings",
			       "org.freedesktop.NetworkManager.Settings",
			       QDBusConnection::systemBus())
			.call("ListConnections");

	if (connsReply.isValid()) {
		foreach(QDBusObjectPath path, connsReply.value()) {
			if (path.path() == "/")
				continue;
			CutieNetworkConnection *nconn =
				new CutieNetworkConnection(this);
			nconn->setPath(path.path());
			if (qdbus_cast<QString>(
				    qdbus_cast<QMap<QString, QVariant> >(
					    nconn->data().value("connection"))
					    .value("type")) ==
			    "802-11-wireless") {
				d->m_savedConnections.insert(path, nconn);
				emit savedConnectionsChanged(
					savedConnections());
			}
		}
	}

	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager",
		"/org/freedesktop/NetworkManager",
		"org.freedesktop.DBus.Properties", "PropertiesChanged", d,
		SLOT(onPropertiesChanged(QString, QMap<QString, QVariant>,
					 QStringList)));

	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager",
		"/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "DeviceAdded", d,
		SLOT(onDeviceAdded(QDBusObjectPath)));

	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager",
		"/org/freedesktop/NetworkManager",
		"org.freedesktop.NetworkManager", "DeviceRemoved", d,
		SLOT(onDeviceRemoved(QDBusObjectPath)));

	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager",
		"/org/freedesktop/NetworkManager/Settings",
		"org.freedesktop.NetworkManager.Settings", "NewConnection", d,
		SLOT(onNewConnection(QDBusObjectPath)));

	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager",
		"/org/freedesktop/NetworkManager/Settings",
		"org.freedesktop.NetworkManager.Settings", "ConnectionRemoved",
		d, SLOT(onConnectionRemoved(QDBusObjectPath)));
}

WifiSettings::~WifiSettings()
{
	Q_D(WifiSettings);
	delete d;
}

QList<WifiAccessPoint *> WifiSettings::accessPoints()
{
	Q_D(WifiSettings);
	QList<WifiAccessPoint *> aps = d->m_accessPoints.values();
	std::sort(
		aps.begin(), aps.end(),
		[](WifiAccessPoint *const a, WifiAccessPoint *const b) -> bool {
			return (qdbus_cast<uint>(a->data().value("Strength")) >
				qdbus_cast<uint>(b->data().value("Strength")));
		});
	return aps;
}

WifiAccessPoint *WifiSettings::activeAccessPoint()
{
	Q_D(WifiSettings);
	return d->m_accessPoints.value(d->m_activeAccessPoint);
}

QList<CutieNetworkConnection *> WifiSettings::savedConnections()
{
	Q_D(WifiSettings);
	return d->m_savedConnections.values();
}

void WifiSettings::requestScan()
{
	Q_D(WifiSettings);
	QDBusInterface("org.freedesktop.NetworkManager", d->m_wlanPath.path(),
		       "org.freedesktop.NetworkManager.Device.Wireless",
		       QDBusConnection::systemBus())
		.call("RequestScan", QMap<QString, QVariant>());
}

void WifiSettings::activateConnection(CutieNetworkConnection *connection,
				      WifiAccessPoint *ap)
{
	Q_D(WifiSettings);
	QDBusInterface("org.freedesktop.NetworkManager",
		       "/org/freedesktop/NetworkManager",
		       "org.freedesktop.NetworkManager",
		       QDBusConnection::systemBus())
		.call("ActivateConnection", QDBusObjectPath(connection->path()),
		      QDBusObjectPath(d->m_wlanPath.path()),
		      QDBusObjectPath(ap ? ap->path() : "/"));
}

void WifiSettings::addAndActivateConnection(WifiAccessPoint *ap, QString psk)
{
	Q_D(WifiSettings);
	QMap<QString, QMap<QString, QVariant> > data;

	QMap<QString, QVariant> wireless;
	wireless.insert("id", qdbus_cast<QString>(ap->data().value("Ssid")));
	data.insert("connection", wireless);

	if (psk != "") {
		QMap<QString, QVariant> security;
		security.insert("psk", psk);
		data.insert("802-11-wireless-security", security);
	}

	QDBusInterface("org.freedesktop.NetworkManager",
		       "/org/freedesktop/NetworkManager",
		       "org.freedesktop.NetworkManager",
		       QDBusConnection::systemBus())
		.call("AddAndActivateConnection",
		      QVariant::fromValue<
			      QMap<QString, QMap<QString, QVariant> > >(data),
		      QDBusObjectPath(d->m_wlanPath.path()),
		      QDBusObjectPath(ap->path()));
}

bool WifiSettings::wirelessEnabled()
{
	Q_D(WifiSettings);
	return d->m_wirelessEnabled;
}

void WifiSettings::setWirelessEnabled(bool wirelessEnabled)
{
	Q_D(WifiSettings);
	d->m_wirelessEnabled = wirelessEnabled;
	QDBusMessage message = QDBusMessage::createMethodCall(
		"org.freedesktop.NetworkManager",
		"/org/freedesktop/NetworkManager",
		"org.freedesktop.DBus.Properties", "Set");
	QList<QVariant> arguments;
	arguments << "org.freedesktop.NetworkManager" << "WirelessEnabled"
		  << QVariant::fromValue(QDBusVariant(d->m_wirelessEnabled));
	message.setArguments(arguments);
	QDBusConnection::systemBus().call(message);
}

QObject *WifiSettings::provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
	Q_UNUSED(scriptEngine);
	return new WifiSettings(engine);
}

WifiSettingsPrivate::WifiSettingsPrivate(WifiSettings *q)
	: q_ptr(q)
{
}

WifiSettingsPrivate::~WifiSettingsPrivate()
{
}

void WifiSettingsPrivate::onNewConnection(QDBusObjectPath path)
{
	Q_Q(WifiSettings);
	CutieNetworkConnection *nconn = new CutieNetworkConnection(this);
	nconn->setPath(path.path());
	m_savedConnections.insert(path, nconn);
	emit q->savedConnectionsChanged(q->savedConnections());
}

void WifiSettingsPrivate::onConnectionRemoved(QDBusObjectPath path)
{
	Q_Q(WifiSettings);
	m_savedConnections.remove(path);
	emit q->savedConnectionsChanged(q->savedConnections());
}

void WifiSettingsPrivate::onDeviceAdded(QDBusObjectPath path)
{
	Q_Q(WifiSettings);
	if (m_wlanPath.path() != "")
		return;
	QDBusReply<QVariantMap> devPropReply =
		QDBusInterface("org.freedesktop.NetworkManager", path.path(),
			       "org.freedesktop.DBus.Properties",
			       QDBusConnection::systemBus())
			.call("GetAll",
			      "org.freedesktop.NetworkManager.Device");

	if (devPropReply.isValid()) {
		QVariantMap devProps = devPropReply.value();
		uint type = devProps["DeviceType"].value<uint>();
		if (type == 2) // Wireless LAN
			m_wlanPath = path;
	}

	if (m_wlanPath.path() == "")
		return;

	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager", m_wlanPath.path(),
		"org.freedesktop.NetworkManager.Device.Wireless",
		"AccessPointAdded", this,
		SLOT(onAccessPointAdded(QDBusObjectPath)));

	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager", m_wlanPath.path(),
		"org.freedesktop.NetworkManager.Device.Wireless",
		"AccessPointRemoved", this,
		SLOT(onAccessPointRemoved(QDBusObjectPath)));

	QDBusConnection::systemBus().connect(
		"org.freedesktop.NetworkManager", m_wlanPath.path(),
		"org.freedesktop.DBus.Properties", "PropertiesChanged", this,
		SLOT(onDevicePropertiesChanged(QString, QMap<QString, QVariant>,
					       QStringList)));

	QDBusReply<QVariantMap> wlanPropReply =
		QDBusInterface("org.freedesktop.NetworkManager",
			       m_wlanPath.path(),
			       "org.freedesktop.DBus.Properties",
			       QDBusConnection::systemBus())
			.call("GetAll",
			      "org.freedesktop.NetworkManager.Device.Wireless");

	if (wlanPropReply.isValid()) {
		QVariantMap wlanProps = wlanPropReply.value();
		QList<QDBusObjectPath> aps =
			qdbus_cast<QList<QDBusObjectPath> >(
				wlanProps["AccessPoints"]);
		foreach(QDBusObjectPath path, aps)
			onAccessPointAdded(path);
		m_activeAccessPoint =
			wlanProps["ActiveAccessPoint"].value<QDBusObjectPath>();
		emit q->activeAccessPointChanged(
			m_accessPoints.value(m_activeAccessPoint));
	}
}

void WifiSettingsPrivate::onDeviceRemoved(QDBusObjectPath path)
{
	Q_Q(WifiSettings);
	if (m_wlanPath.path() == path.path())
		m_wlanPath.setPath("");
}

void WifiSettingsPrivate::onAccessPointAdded(QDBusObjectPath path)
{
	Q_Q(WifiSettings);
	WifiAccessPoint *wifiAP = new WifiAccessPoint(this);
	wifiAP->setPath(path.path());
	m_accessPoints.insert(path, wifiAP);
	emit q->accessPointsChanged(q->accessPoints());
}

void WifiSettingsPrivate::onAccessPointRemoved(QDBusObjectPath path)
{
	Q_Q(WifiSettings);
	m_accessPoints.remove(path);
	emit q->accessPointsChanged(q->accessPoints());
}

void WifiSettingsPrivate::onPropertiesChanged(QString iface,
					      QMap<QString, QVariant> updated,
					      QStringList invalidated)
{
	Q_Q(WifiSettings);
	Q_UNUSED(invalidated);
	if (iface == "org.freedesktop.NetworkManager") {
		if (updated.contains("WirelessEnabled")) {
			m_wirelessEnabled = qdbus_cast<bool>(
				updated.value("WirelessEnabled"));
			emit q->wirelessEnabledChanged(m_wirelessEnabled);
		}
	};
}

void WifiSettingsPrivate::onDevicePropertiesChanged(
	QString iface, QMap<QString, QVariant> updated, QStringList invalidated)
{
	Q_Q(WifiSettings);
	Q_UNUSED(invalidated);
	if (iface == "org.freedesktop.NetworkManager.Device.Wireless") {
		if (updated.contains("ActiveAccessPoint")) {
			m_activeAccessPoint = qdbus_cast<QDBusObjectPath>(
				updated.value("ActiveAccessPoint"));
			emit q->activeAccessPointChanged(
				m_accessPoints.value(m_activeAccessPoint));
		}
	};
}
