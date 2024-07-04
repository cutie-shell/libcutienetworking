#pragma once
#include <QtQuick>

class MobileNetworkPrivate;
class CutieNetworkConnectionPrivate;
class WifiAccessPointPrivate;
class WifiSettingsPrivate;

class MobileNetwork : public QObject {
	Q_OBJECT
	Q_DECLARE_PRIVATE(MobileNetwork)
	Q_PROPERTY(bool mobileDataEnabled READ mobileDataEnabled WRITE
			   setMobileDataEnabled NOTIFY mobileDataEnabledChanged)
	Q_PROPERTY(bool cellularEnabled READ cellularEnabled WRITE
			   setCellularEnabled NOTIFY cellularEnabledChanged)
	Q_PROPERTY(QVariantList availableConnections READ availableConnections
			   NOTIFY availableConnectionsChanged)
	Q_PROPERTY(QString activeConnection READ activeConnection WRITE
			   setActiveConnection NOTIFY activeConnectionChanged)

    public:
	MobileNetwork(QObject *parent = 0);
	~MobileNetwork();

	bool mobileDataEnabled();
	bool cellularEnabled();
	QVariantList availableConnections();
	QString activeConnection();

	void setMobileDataEnabled(bool mobileDataEnabled);
	void setCellularEnabled(bool cellularEnabled);
	void setActiveConnection(QString path);
	Q_INVOKABLE void addAndActivateConnection(QString con_name,
						  QString apn);
	Q_INVOKABLE QString addConnection(QString con_name, QString apn);
	Q_INVOKABLE void updateConnection(QString path, QString con_name,
					  QString apn);
	Q_INVOKABLE void deleteConnection(QString path);

	static QObject *provider(QQmlEngine *engine, QJSEngine *scriptEngine);

    signals:
	void mobileDataEnabledChanged(bool);
	void availableConnectionsChanged();
	void cellularEnabledChanged(bool);
	void activeConnectionChanged(QString);

    private:
	MobileNetworkPrivate *d_ptr;
};

class CutieNetworkConnection : public QObject {
	Q_OBJECT
	Q_DECLARE_PRIVATE(CutieNetworkConnection)
	Q_PROPERTY(QString path READ path() NOTIFY pathChanged);
	Q_PROPERTY(QMap<QString, QVariant> data READ data() NOTIFY dataChanged);

    public:
	CutieNetworkConnection(QObject *parent = 0);
	~CutieNetworkConnection();

	QString path();
	void setPath(QString path);
	QMap<QString, QVariant> data();

	Q_INVOKABLE void deleteConnection();

    signals:
	void pathChanged(QString path);
	void dataChanged(QMap<QString, QVariant> data);

    private:
	CutieNetworkConnectionPrivate *d_ptr;
};

class WifiAccessPoint : public QObject {
	Q_OBJECT
	Q_DECLARE_PRIVATE(WifiAccessPoint)
	Q_PROPERTY(QString path READ path() NOTIFY pathChanged);
	Q_PROPERTY(QMap<QString, QVariant> data READ data() NOTIFY dataChanged);

    public:
	WifiAccessPoint(QObject *parent = 0);
	~WifiAccessPoint();

	QString path();
	void setPath(QString path);
	QMap<QString, QVariant> data();

    signals:
	void pathChanged(QString path);
	void dataChanged(QMap<QString, QVariant> data);

    private:
	WifiAccessPointPrivate *d_ptr;
};

class WifiSettings : public QObject {
	Q_OBJECT
	Q_DECLARE_PRIVATE(WifiSettings)
	Q_PROPERTY(QList<WifiAccessPoint *> accessPoints READ accessPoints
			   NOTIFY accessPointsChanged)
	Q_PROPERTY(WifiAccessPoint *activeAccessPoint READ activeAccessPoint
			   NOTIFY activeAccessPointChanged)
	Q_PROPERTY(QList<CutieNetworkConnection *> savedConnections READ
			   savedConnections NOTIFY savedConnectionsChanged)
	Q_PROPERTY(bool wirelessEnabled READ wirelessEnabled WRITE
			   setWirelessEnabled NOTIFY wirelessEnabledChanged)
    public:
	WifiSettings(QObject *parent = 0);
	~WifiSettings();

	QList<WifiAccessPoint *> accessPoints();
	WifiAccessPoint *activeAccessPoint();
	QList<CutieNetworkConnection *> savedConnections();
	bool wirelessEnabled();

	void setWirelessEnabled(bool wirelessEnabled);

	Q_INVOKABLE void requestScan();
	Q_INVOKABLE void activateConnection(CutieNetworkConnection *connection,
					    WifiAccessPoint *ap);
	Q_INVOKABLE void addAndActivateConnection(WifiAccessPoint *ap,
						  QString psk);

	static QObject *provider(QQmlEngine *engine, QJSEngine *scriptEngine);

    signals:
	void accessPointsChanged(QList<WifiAccessPoint *>);
	void activeAccessPointChanged(WifiAccessPoint *);
	void savedConnectionsChanged(QList<CutieNetworkConnection *>);
	void wirelessEnabledChanged(bool);

    private:
	WifiSettingsPrivate *d_ptr;
};
