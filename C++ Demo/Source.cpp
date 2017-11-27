#include <iostream>
#include "Cards.h"

using namespace Cards;

/// <summary>
/// Handle Card Tap event. This event is raised when a user taps his phone on the reader. 
/// Note that Cards must be installed on the phone.
/// </summary>
void __stdcall HandleCardTap(CardTapResponse cardInfo)
{
	if (!cardInfo.IsSuccess)
	{
		std::cout << "Failed reading card, error code: " << cardInfo.Error << std::endl;
	}
	else
	{
		std::cout << "Card read, user ID: " << cardInfo.CardDetails.UserID << std::endl;
		//Your code goes here!
		//Do whatever you want with the accepted User ID!

		//-----------------------
		//Example: Open the door, if the user is authorized
		//-----------------------
		/*if (YourSystem.IsAuthorizedToOpenDoor(cardInfo.CardDetails.UserID, Doors.Hallway))
		{
		YourSystem.OpenDoor(Doors.Hallway);
		}
		*/

		//-----------------------
		//Example: Remove balance
		//-----------------------
		/*
		YourSystem.Users.ChangeBalance(cardInfo.CardDetails.UserID, -10);
		*/
	}
}

/// <summary>
/// Handles reader status change. 
/// </summary>
void __stdcall HandleStatusChange(ReaderStatus readerStatus)
{
	switch (readerStatus)
	{
	case ReaderStatus::Disconnected:
		std::cout << "Card reader has been disconnected!" << std::endl;;
		break;
	case ReaderStatus::Connected:
		std::cout << "Card reader has been connected!" << std::endl;
		break;
	}
}

void main()
{
	//Create reader settings
	ReaderSettings readerSettings("ACS - ACR122U PICC Interface");

	//Create reader credentials
	ReaderCredentials readerCredentials("ABCD1234ABCD1234ABCD1234ABCD1234");

	//Create a card reader
	CardReader cardReader(readerSettings, readerCredentials);

	//Subscribe to Card Tap event
	cardReader.OnCardTap = HandleCardTap;

	//Subscribe to Status Change event
	cardReader.OnStatusChange = HandleStatusChange;

	//Start listening!
	cardReader.Listen();
}