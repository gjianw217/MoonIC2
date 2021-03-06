RhB!
�2�b��!�5�"��B1�#��!����	4%�!�C$���d��!�Ba�(M �  ��h,@���P)�1P9���M��Bq	�3��y	�(͡�����4hP�!�C\����T:�8/_Օ�L�j>�0�>��������߉��&3|iq,c��u�X'�Nc&�h��_��a$>��HL<�ve�>b�{ \�X,W��c���e��l�qԌ/�/����/��n��%,�|�M�h��iܤ�GC��n摇|�ڃz>�yh��s���*�b��lr]8�L6O��������~��U�V����5�E�U�e�u����������������5�U�e���|��į�/�_�|:�Z�b���+�ů�_�9|��z��K���/�/�_,�]~M~Q~U|����K�/x��^2�׫^�y�/%yk�^j�*/����������������W�����W�����W�����W��������������ߪ\���N�X�`!�������br�5>�W�/P�%6���ļ#��:��7��2C�����ѩw��)�)�)5lLL�3�>�L�&�OPq�sqso���K%��	7%	!�D���z�����tT�H�k`ȃ���ߊ�W�S�S�ݩ��)��T֠��k�X�.��	����@ �?�� ),:5 ���H���̘I��@3��Π@����0��12��rg^wfL͙��	�Χ$���:��7x��M���=���0"N������"�����!��#Z>1h+JP+����|��<�|�-O�w����?ٙ3@�-{���}?����}��d_��߲UVտ���X,�����|�23���6Bp>p�	��'ÎB���A�H}!F��
P�Xs�9��*�?�U89�V ��V�pW��X��Y�H:ah���[X-��:�r���v���/�/��_��v�``E+)ЬQN�;�c
	QString QextSerialEnumerator::getRegKeyValue(HKEY key, LPCTSTR property)
	{
		DWORD size = 0;
		RegQueryValueEx(key, property, NULL, NULL, NULL, & size);
		BYTE * buff = new BYTE[size];
		if (RegQueryValueEx(key, property, NULL, NULL, buff, & size) == ERROR_SUCCESS) {
			return TCHARToQStringN(buff, size);
			delete [] buff;
		} else {
			qWarning("QextSerialEnumerator::getRegKeyValue: can not obtain value from registry");
			delete [] buff;
			return QString();
		}
	}
	
	//static
	QString QextSerialEnumerator::getDeviceProperty(HDEVINFO devInfo, PSP_DEVINFO_DATA devData, DWORD property)
	{
		DWORD buffSize = 0;
		SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, NULL, 0, & buffSize);
		BYTE * buff = new BYTE[buffSize];
		if (!SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, buff, buffSize, NULL))
			qCritical("Can not obtain property: %ld from registry", property); 
		QString result = TCHARToQString(buff);
		delete [] buff;
		return result;
	}

	//static
	void QextSerialEnumerator::setupAPIScan(QList<QextPortInfo> & infoList)
	{
		HDEVINFO devInfo = INVALID_HANDLE_VALUE;
		GUID * guidDev = (GUID *) & GUID_CLASS_COMPORT;

		devInfo = SetupDiGetClassDevs(guidDev, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if(devInfo == INVALID_HANDLE_VALUE) {
			qCritical("SetupDiGetClassDevs failed. Error code: %ld", GetLastError());
			return;
		}

		//enumerate the devices
		bool ok = true;
		SP_DEVICE_INTERFACE_DATA ifcData;
		ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		SP_DEVICE_INTERFACE_DETAIL_DATA * detData = NULL;
		DWORD detDataSize = 0;
		DWORD oldDetDataSize = 0;
		
		for (DWORD i = 0; ok; i++) {
			ok = SetupDiEnumDeviceInterfaces(devInfo, NULL, guidDev, i, &ifcData);
			if (ok) {
				SP_DEVINFO_DATA devData = {sizeof(SP_DEVINFO_DATA)};
				//check for required detData size
				SetupDiGetDeviceInterfaceDetail(devInfo, & ifcData, NULL, 0, & detDataSize, & devData);
				//if larger than old detData size then reallocate the buffer
				if (detDataSize > oldDetDataSize) {
					delete [] detData;
					detData = (SP_DEVICE_INTERFACE_DETAIL_DATA *) new char[detDataSize];
					detData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
					oldDetDataSize = detDataSize;
				}
				//check the details
				if (SetupDiGetDeviceInterfaceDetail(devInfo, & ifcData, detData, detDataSize, 
													NULL, & devData)) {
					// Got a device. Get the details.
					QextPortInfo info;
					info.friendName = getDeviceProperty(devInfo, & devData, SPDRP_FRIENDLYNAME);
					info.physName = getDeviceProperty(devInfo, & devData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME);
					info.enumName = getDeviceProperty(devInfo, & devData, SPDRP_ENUMERATOR_NAME);
					//anyway, to get the port name we must still open registry directly :( ??? 
					//Eh...			
					HKEY devKey = SetupDiOpenDevRegKey(devInfo, & devData, DICS_FLAG_GLOBAL, 0,
														DIREG_DEV, KEY_READ);
					info.portName = getRegKeyValue(devKey, TEXT("PortName"));
					RegCloseKey(devKey);
					infoList.append(info);
				} else {
					qCritical("SetupDiGetDeviceInterfaceDetail failed. Error code: %ld", GetLastError());
					delete [] detData;
					return;
				}
			} else {
				if (GetLastError() != ERROR_NO_MORE_ITEMS) {
					delete [] detData;
					qCritical("SetupDiEnumDeviceInterfaces failed. Error code: %ld", GetLastError());
					return;
				}
			}
		}
		delete [] detData;
	}

#endif /*_TTY_WIN_*/


//static
QList<QextPortInfo> QextSerialEnumerator::getPorts()
{
	QList<QextPortInfo> ports;

	#ifdef _TTY_WIN_
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (!::GetVersionEx(&vi)) {
			qCritical("Could not get OS version.");
			return ports;
		}
		// Handle windows 9x and NT4 specially
		if (vi.dwMajorVersion < 5) {
			qCritical("Enumeration for this version of Windows is not implemented yet");
/*			if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
				EnumPortsWNt4(ports);
			else
				EnumPortsW9x(ports);*/
		} else	//w2k or later
			setupAPIScan(ports);
	#endif /*_TTY_WIN_*/
	#ifdef _TTY_POSIX_
		qCritical("Enumeration for POSIX systems is not implemented yet.");
	#endif /*_TTY_POSIX_*/
	
	return ports;
}
