#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>

const uintptr_t billiardAppRefOffset = 0x155B2C;
const uintptr_t playerCameraOffset = 0x1D0;

const uintptr_t ballPosOffset = 0x3C;
const uintptr_t ballNumOffset = 0x54;
const uintptr_t cameraPosOffset = 0x120;

uintptr_t GetBallByNumber(uintptr_t billiardApp, int targetNum)
{
	for (int i = 0; i < 16; i++)
	{
		// 8B 8E ?? ?? ?? ?? 8B 14 B9 33 D7
		// i just copied this from the assembly instructions
		uintptr_t ecx = *(uintptr_t*)(billiardApp + 0x235C);
		uintptr_t edx = *(uintptr_t*)(ecx + (i * 4));
		edx ^= i;
		edx ^= *(uintptr_t*)(billiardApp + 0x236C);

		uintptr_t ball = *(uintptr_t*)(edx);

		int num = *(int*)(ball + ballNumOffset);
		if (num == targetNum)
		{
			return ball;
		}
	}
	
	return 0;
}

void UpdateConsole(int* balls, int selected, int pocket, int combinations, bool doBankShot, bool bankingCue, int selectedBankWall, float targetAngle, float currentAngle)
{
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 12 });

	std::cout << "Target Pocket: " << pocket << "           \n";
	std::cout << "\n(editing ball " << (selected + 1) << ")                  \n";
	for (int i = 0; i <= combinations; i++)
	{
		std::cout << "Target Ball " << (i + 1) << ": " << balls[i] << "           \n";
	}

	if (doBankShot) 
	{
		std::cout << "\nSelected Bank Wall: " << selectedBankWall << "                         \n";

		if (bankingCue) 
		{
			std::cout << "Hitting the cue ball off this wall.                 \n";
		}
		else 
		{
			std::cout << "Hitting the target ball off this wall.              \n";
		}
	}

	std::cout << "                       \nTarget Angle: " << targetAngle << "                      \n";
	std::cout << "Current Angle: " << currentAngle << "                   \n";

	std::cout << "                                       \n";
	std::cout << "                                       \n";
	std::cout << "                                       \n";
	std::cout << "                                       \n";
	std::cout << "                                       \n";
}

DWORD WINAPI Thread(LPVOID param)
{
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	// print base text
	std::cout << "Key Binds: \n"; 
	std::cout << "ins - Exit\n";
	std::cout << "R - Reset (if angles are off)\n";
	std::cout << "M - Increase number of target balls for a combination shot. Target ball 1 is the one you want to pocket.\n";
	std::cout << ", - Change selected ball to edit if their are multible target balls\n";
	std::cout << "0 - Toggle between selecting stripes vs solids\n";
	std::cout << "1-8 - Select balls 1 - 8, or 9 - 15 if 0 is pressed\n";
	std::cout << "Z-N - Select pocket where 1 is the bottom right (looking at the couches); going clockwise\n";

	std::cout << "L - Toggle bank shot\n";
	std::cout << "K - Toggle bank shot with cue ball\n";
	std::cout << "WASD - Select bank shot wall (look towards couches)\n\n";
	
	uintptr_t gameDllBase = (uintptr_t)GetModuleHandle(L"game.dll");
	uintptr_t billiardApp = *(uintptr_t*)(gameDllBase + billiardAppRefOffset);
	uintptr_t playerCamera = *(uintptr_t*)(billiardApp + playerCameraOffset);

	int solidsOrStripes = 0;
	int balls[4] = { 1, 1, 1, 1 }; // balls involved in the combination shot
	int pocketNum = 0;

	int combinations = 0;
	int selectedBallIndex = 0; // the ball to change

	bool doBankShot = false;
	bool bankCueBall = false;
	int selectedBankWall = 0;

	float targetAngle = 0;
	float currentAngle = 0;

	const float pockets[6][2] =
	{
		{-0.525, 1}, // bottom right when looking at the couches. rest go clockwise from there
		{-0.55, 0},
		{-0.525, -1},
		{0.525, -1},
		{0.55, 0},
		{0.525, 1}
	};

	// windows key codes:
	// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

	while (!GetAsyncKeyState(0x2D)) // ins
	{
		if (GetAsyncKeyState(0x30) & 1) // 0
		{
			solidsOrStripes = solidsOrStripes == 0 ? 8 : 0; // toggle between them
		}

		if (GetAsyncKeyState(0x31) & 1) { balls[selectedBallIndex] = 1 + solidsOrStripes; } // 1
		else if (GetAsyncKeyState(0x32) & 1) { balls[selectedBallIndex] = 2 + solidsOrStripes; } // 2
		else if (GetAsyncKeyState(0x33) & 1) { balls[selectedBallIndex] = 3 + solidsOrStripes; } // 3
		else if (GetAsyncKeyState(0x34) & 1) { balls[selectedBallIndex] = 4 + solidsOrStripes; } // 4
		else if (GetAsyncKeyState(0x35) & 1) { balls[selectedBallIndex] = 5 + solidsOrStripes; } // 5
		else if (GetAsyncKeyState(0x36) & 1) { balls[selectedBallIndex] = 6 + solidsOrStripes; } // 6
		else if (GetAsyncKeyState(0x37) & 1) { balls[selectedBallIndex] = 7 + solidsOrStripes; } // 7
		else if (GetAsyncKeyState(0x38) & 1) { balls[selectedBallIndex] = 8; } // 8

		if (GetAsyncKeyState(0x5A) & 1) { pocketNum = 0; } // Z
		else if (GetAsyncKeyState(0x58) & 1) { pocketNum = 1; } // X
		else if (GetAsyncKeyState(0x43) & 1) { pocketNum = 2; } // C
		else if (GetAsyncKeyState(0x56) & 1) { pocketNum = 3; } // V
		else if (GetAsyncKeyState(0x42) & 1) { pocketNum = 4; } // B
		else if (GetAsyncKeyState(0x4E) & 1) { pocketNum = 5; } // N

		if (GetAsyncKeyState(0x4C) & 1) { doBankShot = !doBankShot; } // L
		if (GetAsyncKeyState(0x4B) & 1) { bankCueBall = !bankCueBall; } // K

		if (GetAsyncKeyState(0x57) & 1) { selectedBankWall = 2; } // W
		else if (GetAsyncKeyState(0x41) & 1) { selectedBankWall = 1; } // A
		else if (GetAsyncKeyState(0x53) & 1) { selectedBankWall = 0; } // S
		else if (GetAsyncKeyState(0x44) & 1) { selectedBankWall = 3; } // D

		if (GetAsyncKeyState(0x4D) & 1) // M
		{
			combinations++;
			selectedBallIndex = combinations;
			if (combinations > 3) { combinations = 0; selectedBallIndex = 0; }
		}
		if (GetAsyncKeyState(0xBC) & 1) // ,
		{
			selectedBallIndex++;
			if (selectedBallIndex > combinations) { selectedBallIndex = 0; }
		}

		if (GetAsyncKeyState(0x52) & 1) // R
		{
			billiardApp = *(uintptr_t*)(gameDllBase + billiardAppRefOffset);
			playerCamera = *(uintptr_t*)(billiardApp + playerCameraOffset);

			solidsOrStripes = 0;
			balls[0] = 1;
			pocketNum = 0;
			combinations = 0;
			selectedBallIndex = 0;

			doBankShot = false;
			bankCueBall = false;
			selectedBankWall = 0;

			targetAngle = 0;
			currentAngle = 0;
		}
		
		uintptr_t targetBall = GetBallByNumber(billiardApp, balls[0]);
		if (targetBall == 0) 
		{
			break;
		}

		float targetBallPos[2] = { *(float*)(targetBall + ballPosOffset), *(float*)(targetBall + ballPosOffset + 0x8) };

		float pocketPos[2] = { pockets[pocketNum][0], pockets[pocketNum][1] };
		if (doBankShot) // https://www.billiardsthegame.com/finding-the-bank-angle-317
		{
			// reflect the pocket position
			if (bankCueBall) 
			{
				switch (selectedBankWall)
				{
				case 0:
					targetBallPos[0] = (targetBallPos[0] + 0.5375) * -2;
					break;
				case 2:
					targetBallPos[0] = (targetBallPos[0] - 0.5375) * -2;
					break;
				case 1:
					targetBallPos[1] = (targetBallPos[1] + 1) * -1.5;
					break;
				case 3:
					targetBallPos[1] = (targetBallPos[1] - 1) * -1.5;
					break;
				}
			}
			else 
			{
				switch (selectedBankWall)
				{
				case 0:
					pocketPos[0] = (pocketPos[0] + 0.5375) * -2;
					break;
				case 2:
					pocketPos[0] = (pocketPos[0] - 0.5375) * -2;
					break;
				case 1:
					pocketPos[1] = (pocketPos[1] + 1) * -1.5;
					break;
				case 3:
					pocketPos[1] = (pocketPos[1] - 1) * -1.5;
					break;
				}
			}
		}

		float direction[2] = { targetBallPos[0] - pocketPos[0], targetBallPos[1] - pocketPos[1] }; // between target ball and pocket
		float m = sqrt((direction[0] * direction[0]) + (direction[1] * direction[1])); // normalize
		direction[0] /= m;
		direction[1] /= m;

		float targetPosition[2] = { targetBallPos[0] + (direction[0] * 0.0565), targetBallPos[1] + (direction[1] * 0.0565) }; // location where the cue ball needs to hit

		unsigned char failed = 0;
		for (int i = 1; i <= combinations; i++) 
		{
			targetBall = GetBallByNumber(billiardApp, balls[i]);
			if (targetBall == 0)
			{
				failed = 1;
				break;
			}

			targetBallPos[0] = *(float*)(targetBall + 0x3C);
			targetBallPos[1] = *(float*)(targetBall + 0x44);

			direction[0] = targetBallPos[0] - targetPosition[0];
			direction[1] = targetBallPos[1] - targetPosition[1];
			m = sqrt((direction[0] * direction[0]) + (direction[1] * direction[1])); // normalize
			direction[0] /= m;
			direction[1] /= m;

			targetPosition[0] = targetBallPos[0] + (direction[0] * 0.0565);
			targetPosition[1] = targetBallPos[1] + (direction[1] * 0.0565);
		}

		if (failed)
		{
			break;
		}

		uintptr_t cueBall = GetBallByNumber(billiardApp, 0);
		if (cueBall == 0)
		{
			break;
		}

		float cueBallPos[2] = { *(float*)(cueBall + 0x3C), *(float*)(cueBall + 0x44) };

		float targetDirection[2] = { targetPosition[0] - cueBallPos[0], targetPosition[1] - cueBallPos[1] };
		m = sqrt((targetDirection[0] * targetDirection[0]) + (targetDirection[1] * targetDirection[1])); // normalize
		targetDirection[0] /= m;
		targetDirection[1] /= m;

		targetAngle = atan2(targetDirection[0], targetDirection[1]) * 180 / 3.14159;

		float camX = *(float*)(playerCamera + cameraPosOffset);
		float camY = *(float*)(playerCamera + cameraPosOffset + 0x8);

		currentAngle = atan2(camX, camY) * 180 / 3.14159;

		UpdateConsole(balls, selectedBallIndex, pocketNum + 1, combinations, doBankShot, bankCueBall, selectedBankWall, targetAngle, currentAngle);
	}

	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread((HMODULE)param, 0);

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH) 
	{
		CreateThread(0, 0, Thread, hModule, 0, 0);
	}

	return TRUE;
}