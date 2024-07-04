#pragma once
#include <QDebug>
#include <QtQuick>
#include <QtQml/qqml.h>
#include <QtQml/QQmlExtensionPlugin>

#include "cutienetworking.h"

class CutieNetworkingPlugin : public QQmlExtensionPlugin {
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid FILE
			  "cutienetworking.json")
    public:
	explicit CutieNetworkingPlugin()
	{
	}

	void registerTypes(const char *uri) override;
};