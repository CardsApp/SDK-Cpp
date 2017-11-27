#pragma once
#include <Windows.h>
#include <string>
#include <vector>

namespace Cards
{
	struct DevicesList
	{
		char** Cstrs;
		int Count;
	};

	struct TransferResponse
	{
		int ErrorCode;
		int TransferredLength;
	};

	struct CardTapResponseStruct
	{
		int IsSuccess;
		int ErrorCode;
		char Uid[32];
	};

	enum CardTapError : UINT16
	{
		InternetError = 1,
		ApiKeyInvalid = 2,
		TransactionTokenInvalid = 3,
		UserNotAssociatedWithCardReader = 4,
		NoSuchTPID = 5,
		TransactionTokenMissing = 6,
		TransactionTokenDoesntExist = 7,
		TransactionTokenAlreadyUsed = 8,
		TransactionTokenAlreadyValidated = 9
	};

	enum ReaderStatus : int
	{
		Disconnected = 1,
		Connected = 2,
		AlreadyInUse = 3
	};

	class ReaderCredentials
	{
	public:
		std::string ApiKey; //Access token to Cards API

		ReaderCredentials()
		{

		}

		ReaderCredentials(std::string ApiKey)
		{
			this->ApiKey = ApiKey;
		}
	};

	class ReaderSettings
	{
	public:
		std::string DeviceName; //The actual USB device name, i.e. "ACS - ACR122U PICC Interface"

		ReaderSettings()
		{
		}

		ReaderSettings(std::string deviceName)
		{
			this->DeviceName = deviceName;
		}
	};

	class CardDetails
	{
	public:
		std::string UserID; //Third party User ID, as resolved by Cards API
	};

	class CardTapResponse
	{
	public:
		bool IsSuccess = false;
		CardTapError Error;
		CardDetails CardDetails;
	};

	typedef void(__stdcall *InternalCardTapHandler)(void* obj, CardTapResponseStruct CardInfo);
	typedef void(__stdcall *CardTapHandler)(CardTapResponse CardInfo);
	typedef void(__stdcall *StatusChangeHandler)(ReaderStatus status);

	typedef BOOL(__stdcall *f_createInstanceByName)(void** Instance, char* DeviceName, char* ApiKey);
	typedef BOOL(__stdcall *f_destroyInstance)(void* Instance);
	typedef void(__stdcall *f_runOnCardPresent)(void* Instance, void* obj, InternalCardTapHandler OnCardTap, StatusChangeHandler OnStatusChange);
	typedef BOOL(__stdcall *f_freeDevicesList)(DevicesList list);
	typedef DevicesList(__stdcall *f_getDevicesList)();

	class NativeBaseDriver
	{
	public:
		static void init()
		{
			dllHandle = LoadLibrary("CardsBase.dll");
			if (!dllHandle)
			{
				throw "Error Loading CardsBase.dll";
			}

			createInstanceByName = (f_createInstanceByName)GetProcAddress(dllHandle, "_createInstanceByName@12");
			destroyInstance = (f_destroyInstance)GetProcAddress(dllHandle, "_destroyInstance@4");
			runOnCardPresent = (f_runOnCardPresent)GetProcAddress(dllHandle, "_runOnCardPresentObj@16");
			freeDevicesList = (f_freeDevicesList)GetProcAddress(dllHandle, "_freeDevicesList@8");
			getDevicesList = (f_getDevicesList)GetProcAddress(dllHandle, "_getDevicesList@0");
		}

		static f_createInstanceByName createInstanceByName;
		static f_destroyInstance destroyInstance;
		static f_runOnCardPresent runOnCardPresent;
		static f_freeDevicesList freeDevicesList;
		static f_getDevicesList getDevicesList;

	private:
		static HMODULE dllHandle;
	};

	class NativeBaseDriverInitializer
	{
	public:
		NativeBaseDriverInitializer()
		{
			NativeBaseDriver::init();
		}
	};

	class CardReader
	{
	public:
		//Ctors
		CardReader(ReaderSettings readerSettings, ReaderCredentials readerCredentials)
		{
			this->ReaderSettings = readerSettings;
			this->ReaderCredentials = readerCredentials;

			if (!NativeBaseDriver::createInstanceByName(&this->_Instance, (char*)this->ReaderSettings.DeviceName.c_str(), (char*)this->ReaderCredentials.ApiKey.c_str()))
			{
				this->_Instance = nullptr;
			}
		}

		CardReader(ReaderCredentials readerCredentials)
		{
			this->ReaderSettings.DeviceName = CardReader::GetDevicesNames()[0];
			this->ReaderCredentials = readerCredentials;

			if (!NativeBaseDriver::createInstanceByName(&this->_Instance, (char*)this->ReaderSettings.DeviceName.c_str(), (char*)this->ReaderCredentials.ApiKey.c_str()))
			{
				this->_Instance = nullptr;
			}
		}

		~CardReader()
		{
			if (this->_Instance != nullptr)
			{
				NativeBaseDriver::destroyInstance(this->_Instance);
			}
		}

		//Methods
		void Listen()
		{
			NativeBaseDriver::runOnCardPresent(this->_Instance, this, &CardReader::InternalCardTapHandler/*this->InternalCardTapHandler*/, /*&CardReader::test2*/this->OnStatusChange);
		}

		static std::vector<std::string> GetDevicesNames()
		{
			std::vector<std::string> devicesNames;

			DevicesList availableDevices = NativeBaseDriver::getDevicesList();

			void* Cstrs = (availableDevices.Cstrs + 8);

			for (int i = 0; i < availableDevices.Count; i++)
			{
				char* Cstr = (char*)(availableDevices.Cstrs + i * 4);
				devicesNames.push_back(std::string(Cstr));
			}

			NativeBaseDriver::freeDevicesList(availableDevices);

			return devicesNames;
		}

		//Properties
		ReaderSettings ReaderSettings;
		ReaderCredentials ReaderCredentials;

		//Events
		CardTapHandler OnCardTap;
		StatusChangeHandler OnStatusChange;

	private:
		static void __stdcall InternalCardTapHandler(void* obj, CardTapResponseStruct CardInfo)
		{
			CardTapResponse response;

			response.IsSuccess = CardInfo.IsSuccess;
			response.Error = (CardTapError)CardInfo.ErrorCode;
			response.CardDetails.UserID = std::string((char*)CardInfo.Uid, 24);


			((CardReader*)obj)->OnCardTap(response);
		}

		void* _Instance;

		NativeBaseDriverInitializer init;
	};
}

Cards::f_createInstanceByName Cards::NativeBaseDriver::createInstanceByName;
Cards::f_destroyInstance Cards::NativeBaseDriver::destroyInstance;
Cards::f_runOnCardPresent Cards::NativeBaseDriver::runOnCardPresent;
Cards::f_freeDevicesList Cards::NativeBaseDriver::freeDevicesList;
Cards::f_getDevicesList Cards::NativeBaseDriver::getDevicesList;
HMODULE Cards::NativeBaseDriver::dllHandle;