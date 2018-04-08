#!/bin/sh
qdbuscpp2xml -M qpaneldbus.h -o com.mooncell.panel.xml
qdbusxml2cpp com.mooncell.panel.xml -i qpaneldbus.h -c QPanelAdaptor -a qpaneladaptor
qdbusxml2cpp com.mooncell.panel.xml -i qpaneldbus.h -c QPanelInterface -p qpanelinterface

