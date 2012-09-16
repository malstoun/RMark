// Limin.cpp : Defines the entry point for the console application.
#include <windows.h>
#include "Limin.h"
#include "IOBMP.h"
#include "PImage.h"
#include <fstream>
#include <string.h>
#include <ctime>

int main(int argc, char *argv[])
{
    IOBMP worker;
	BMPImage img;
	float t = 0;

	if (argc > 1)
	{
		char *namebmp = new char[30];
		int numberImage = 0;
		char widthAr[2];
		char heightAr[2];
		uint8_t* RgbArray;
		int32_t width, height, rowLength;
		uint8_t tempByte;


		HANDLE pipe = CreateNamedPipe(
			L"\\\\.\\pipe\\Limpipe", 
			PIPE_ACCESS_INBOUND, 
			PIPE_TYPE_BYTE, 
			1, 
			0, 
			0, 
			0, 
			NULL 
		);

		if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) 
		{
			wcout << "Failed to create outbound pipe instance.";
			wcout << GetLastError() << endl;
			// look up error code here using GetLastError()
			system("pause");
			return 1;
		}

		HANDLE pipeRegImage = CreateNamedPipe(
			L"\\\\.\\pipe\\LimpipeReg", 
			PIPE_ACCESS_OUTBOUND, 
			PIPE_TYPE_BYTE, 
			1, 
			0, 
			0, 
			0, 
			NULL 
		);

		if (pipeRegImage == NULL || pipeRegImage == INVALID_HANDLE_VALUE) 
		{
			wcout << "Failed to create outbound pipe instance.";
			wcout << GetLastError() << endl;
			// look up error code here using GetLastError()
			system("pause");
			return 1;
		}

		wcout << "Waiting for connection..." << endl;

		BOOL result;
		
		result = ConnectNamedPipe(pipe, NULL);

		if (!result) 
		{
			wcout << "Error WTF?." << endl;
			CloseHandle(pipe); // close the pipe
			system("pause");
			return 1;
		}

		result = ConnectNamedPipe(pipeRegImage, NULL);

		if (!result) 
		{
			wcout << "Error WTF? RegPipe" << endl;
			CloseHandle(pipeRegImage); // close the pipe
			system("pause");
			return 1;
		}

		DWORD numBytesRead = 0;
		
		do
		{
			result = ReadFile(pipe,
				widthAr, 
				2, 
				&numBytesRead, 
				NULL 
			); 

			if (!result) 
			{
				wcout << "Error read file." << endl;
				CloseHandle(pipeRegImage);
				CloseHandle(pipe);
				break;
			}

			if (numBytesRead != 2)
				continue;

			// Parse width. First char is higher byte, second - low

			width = widthAr[0] << 8;
			tempByte = widthAr[1];
			width = width|tempByte;

			result = ReadFile(
				pipe,
				heightAr, 
				2, 
				&numBytesRead, 
				NULL 
			); 

			if (!result) 
			{
				wcout << "Error read file." << endl;
				CloseHandle(pipeRegImage);
				CloseHandle(pipe);
				break;
			}

			if (numBytesRead != 2)
				continue;

			height = heightAr[0] << 8;
			tempByte = heightAr[1];
			height = height|tempByte;

			rowLength = 3 * width;
			if (rowLength % 4 != 0)
				rowLength += (4 - rowLength % 4);

			RgbArray = new uint8_t[rowLength*height];

			result = ReadFile(
				pipe,
				RgbArray, 
				rowLength*height, 
				&numBytesRead, 
				NULL
			);

			if (result) 
			{
				wcout << "Success!" << numBytesRead << endl;
			} 
			else 
			{
				wcout << "Error read file." << endl;
				CloseHandle(pipeRegImage);
				CloseHandle(pipe);
				break;
			}

			img.Init(width, height);

			for (int i = 0; i < height; i++)
			{
				for (int j = 0; j < width; j++)
				{
					img.setPixel((height - i - 1), j, (unsigned char)RgbArray[i*rowLength + 3*j + 2], (unsigned char)RgbArray[i*rowLength + 3*j + 1], (unsigned char)RgbArray[i*rowLength + 3*j]);
				}
			}

			numberImage++;
			_itoa(numberImage, namebmp, 10);
			strcat(namebmp, ".bmp");
			worker.setBmp(img, namebmp);

			PImage pImage(img);
			pImage.transBaW();
			t = clock();
			printf("%f\n", (float)t / CLOCKS_PER_SEC);
			if (pImage.getRegImage().isExist())
			{
				//worker.Cut(pImage.getRegImage());
				_itoa(numberImage, namebmp, 10);
				strcat(namebmp, "_reg_plate.bmp");
				worker.setBmp(pImage.getRegImage(), namebmp);
				_itoa(numberImage, namebmp, 10);
				strcat(namebmp, "_loc_reg_plate.bmp");
				worker.setBmp(pImage.getLocRegImage(), namebmp);
			}
			else
			{
				// ???
			}

			_itoa(numberImage, namebmp, 10);
			strcat(namebmp, "_res.bmp");
			worker.setBmp(img, namebmp);

			/**/

			_itoa(numberImage, namebmp, 10);
			strcat(namebmp, "_loc_reg_plate.bmp");

			result = WriteFile(
				pipeRegImage,
				namebmp, 
				strlen(namebmp), 
				&numBytesRead, 
				NULL
			);


		} while (true);
	}
	else
	{
		if (worker.getBmp("simple.bmp") == SUCCESS)
		{
			img.Init(worker.getBmpInfoHeader().biWidth, worker.getBmpInfoHeader().biHeight);
			img.setPixelArray(worker.getPixelArray());
			PImage pImage(img);
			pImage.transBaW();
			t = clock();
			printf("%f\n", (float)t / CLOCKS_PER_SEC);
			if (pImage.getRegImage().isExist())
			{
				//worker.Cut(pImage.getRegImage());
				worker.setBmp(pImage.getRegImage(), "reg_plate.bmp");
			}
			else
			{
				// ???
			}

			worker.setBmp(img, "resultImage.bmp");
		}
		else
		{
			return ERROR_FILE;
		}
	}

    return 0;
}