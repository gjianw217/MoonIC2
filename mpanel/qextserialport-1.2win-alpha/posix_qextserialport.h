���:v�,�7�C�)A���;�$D�^��@:o�w4r�\#NK`!˳t�B�d�ĩ�f��f����5����3zL�0�ѻ�;��2a���-**��S:��ª]^)��0���O��XRQ�Y�l�<�&�Y}�$��n�H��:MR�Gt�V���ڹF*����@!2=���*�sF�f�\.ꅦ�1��7MA�gc���J���gH��(̗�wYQU �� s�jdڎhQ��V�l/=�=9UEX�����:ڒu�����������`dC6���U���B�q�S��w�c���Q�M�g�C1�VE�\Cw	t�n��f�&d۴]��d���h���^�>^�~XR�h��*�$�����_��S'��J���X��VS/��e;P3����P)*(�I�jK�/E�����i�)�V�n�}v_р�I�����8�:Q�Ug��4y��#t9yɜ�/���v�P����|�G�	#$�9���$�5 �:�]<��\E��u�i�n��b:_Sy��{MV����u�#|�o�_2�i9�
	L��<t�,_�n&��0�6�cA�ܥ����u���r��k?}�C��+K��v����ds�÷����L�%���,��(����3��I�O
��h�H4ٯ���;f����g�a��m�QI����e-1@�]a9yL���^)��,|�:i(�Y�XN`7&�����d�<u�>P�c�.N�g�ݍ�/�o��)���#���q7�n9c��юi����Za�	�i��8�hІl�Z��K�U8?wF�x��S��r���<)������h���ߔ����o5k8	MҴ�h��h�fܖ���@4��&q�%�1tX�N��&:^jY��*�7U[�#��o�(����i0�m,��ZAPB�e�5��) ǯ�j��'�H����͊��w�-LV{K��A�uuT���@c�}z�벨�YP��κ�����ʯ���+'�3�a�;���3�ֽ�)��gX���4̺�;pK���R;~^�K�j�osix_QextSerialPort(const QString & name, const PortSettings& settings, QextSerialBase::QueryMode mode = QextSerialBase::Polling);
	    Posix_QextSerialPort& operator=(const Posix_QextSerialPort& s);
	    virtual ~Posix_QextSerialPort();
	
	    virtual void setBaudRate(BaudRateType);
	    virtual void setDataBits(DataBitsType);
	    virtual void setParity(ParityType);
	    virtual void setStopBits(StopBitsType);
	    virtual void setFlowControl(FlowType);
	    virtual void setTimeout(long);

	    virtual bool open(OpenMode mode);
	    virtual void close();
	    virtual void flush();
	
	    virtual qint64 size() const;
	    virtual qint64 bytesAvailable();
	
	    virtual void ungetChar(char c);
	
	    virtual void translateError(ulong error);
	
	    virtual void setDtr(bool set=true);
	    virtual void setRts(bool set=true);
	    virtual ulong lineStatus();

};

#endif
