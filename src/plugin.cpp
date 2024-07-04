#include "plugin.h"

void CutieNetworkingPlugin::registerTypes(const char *uri)
{
	qmlRegisterSingletonType<WifiSettings>(uri, 1, 0, "CutieWifiSettings",
					       &WifiSettings::provider);
	qmlRegisterSingletonType<MobileNetwork>(uri, 1, 0, "CutieMobileNetwork",
						&MobileNetwork::provider);

	qmlRegisterType<WifiAccessPoint>(uri, 1, 0, "CutieWifiAccessPoint");
	qmlRegisterType<CutieNetworkConnection>(uri, 1, 0,
						"CutieNetworkConnection");
}
