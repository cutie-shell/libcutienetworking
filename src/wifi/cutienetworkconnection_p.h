#pragma once
#include <QDebug>
#include <QtQuick>
#include <QtQml/qqml.h>
#include <QtQml/QQmlExtensionPlugin>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include "../cutienetworking.h"

class CutieNetworkConnectionPrivate : public QObject {
	Q_OBJECT
	Q_DECLARE_PUBLIC(CutieNetworkConnection)

    public:
	CutieNetworkConnectionPrivate(CutieNetworkConnection *q);
	~CutieNetworkConnectionPrivate();

    protected slots:
	void onUpdated();

    protected:
	QString m_path;
	QMap<QString, QMap<QString, QVariant> > m_data;

    private:
	CutieNetworkConnection *q_ptr;
};
