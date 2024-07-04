---
title: CutieNetworkConnection QML Type
---

CutieNetworkConnection provides an interface to interact with a network connection saved to [NetworkManager](https://networkmanager.dev/).

Inherits: QObject

### Properties

- [data](#data): object
- [path](#path): string

### Methods

- [deleteConnection()](#deleteconnection)

## Detailed Description

CutieNetworkConnection is a type representing a network connection saved to [NetworkManager](https://networkmanager.dev/). For wireless networks,  [CutieWifiSettings.addAndActivateConnection](wifisettings#addandactivateconnection) can be used to store a new connection and activate it immediately. Also, [CutieWifiSettings.activateConnection](wifisettings#activateconnection) can be used to activate an existing saved connection represented by an object of this type.

## Property Documentation

#### data

object, readonly

Holds a JavaScript object containing the connection properties as given by [NetworkManager](https://networkmanager.dev/) D-Bus service.

#### path

string, readonly

Holds the D-Bus path that is used by [NetworkManager](https://networkmanager.dev/) describe this connection.

## Method Documentation

#### deleteConnection()

Removes this connection from NetworkManager's saved connections. Does not have a return value.
