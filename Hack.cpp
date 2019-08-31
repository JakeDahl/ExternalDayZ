#include "DirectX.h"
#include "World.hpp"
#include <string>
#include <sstream>
#include "common.h"
#include <map>
#include <mutex>
#include <thread>

World world;
extern int Width;
extern int Height;

int renderdistance = 200;
int itemRenderDistance = 100;
uintptr_t centerTargetEnt;
void BulletToHeadThread();

bool bulletsToHead = false;

std::mutex bulletToHeadMutex;

std::map<UINT64, std::string> nameCache;


int distToCenter;
int closestToCenter;

int getDistanceFromCenter(uintptr_t entity, Vec3 w2s)
{
	Vec3 centerScreen((Width / 2), (Height / 2), 0.0f);
	return Distance(centerScreen, w2s);
}

struct nameid
{
	UINT64 pt1;
	UINT64 pt2;
};

std::string getNameFromId(uintptr_t namePointer)
{
	nameid ID = driver.rpm<nameid>(namePointer + 0x10);
	std::map<UINT64, std::string>::iterator it = nameCache.find(ID.pt1 + ID.pt2);

	if (it == nameCache.end())
	{

		int size = driver.rpm<int>(namePointer + 0x8);
		char* name = new char[size];
		driver.rpmRaw(namePointer + 0x10, name, size);
        //Adds name to cached list of objects we don't want to draw.
		if (strstr(name, "Animal") != NULL || strstr(name, "Zmb") != NULL || strstr(name, "Firewood") != NULL || strstr(name, "Barrel") || strstr(name, "Watchtower") || strstr(name, "Wood Pillar") || strstr(name, "Roof") || strstr(name, "Wall") != NULL || strstr(name, "Floor") || strstr(name, "Fireplace") != NULL || strstr(name, "Wire Mesh Barrier") != NULL || strstr(name, "Fence") != NULL)
		{
			std::string text = "";
			nameCache.insert(std::pair<UINT64, std::string>(ID.pt1 + ID.pt2, text));
			delete name;
			return text;
		}

		std::string text = std::string(name);
		delete name;
		nameCache.insert(std::pair<UINT64, std::string>(ID.pt1 + ID.pt2, text));
		std::cout << "Added new item to cache: " << text << std::endl;
		return text;
	}
	else
	{
		return it->second;
	}
}

void BulletToHeadThread()
{
	while (1)
	{
		if (bulletsToHead)
		{
			uintptr_t worldptr = driver.rpm<uintptr_t>(world.dw_world + world.modulebase);

			int bulletTableSize = driver.rpm<int>(worldptr + world.dw_bulletTable + 0x8);

			if (bulletTableSize > 0)
			{
				bulletToHeadMutex.lock();
				uintptr_t target = centerTargetEnt;
				bulletToHeadMutex.unlock();

				Vec3 headpos = world.GetVisualState(target);

				for (size_t i = 0; i < bulletTableSize; i++)
				{
					uintptr_t bullet = driver.rpm<uintptr_t>(worldptr + world.dw_bulletTable + (i * 0x8));
					uintptr_t visualState = driver.rpm<uintptr_t>(bullet + world.dw_objectVisualState);

					driver.wpm<Vec3>(headpos, visualState + 0x2C);
				}
			}

		}
		Sleep(5);
	}
}


void IterateItems(uintptr_t worldptr)
{
	int objectTableSz = driver.rpm<int>(worldptr + world.dw_ItemsOnGround + 0x8);

	std::stringstream all;
	uintptr_t entityTable = driver.rpm<uintptr_t>(worldptr + world.dw_ItemsOnGround);

	//TransData transData = world.GetTransData();
	for (size_t i = 0; i < objectTableSz; i++)
	{
		TransData transData = world.GetTransData();
		if (!entityTable) continue;

		int check = driver.rpm<int>(entityTable + (i * 0x18));

		if (check != 1) continue;

		uintptr_t entity = driver.rpm<uintptr_t>(entityTable + ((i * 0x18) + 0x8));
		if (entity)
		{
			Vec3 Pos = world.GetObjectVisualState(entity);

			Vec3 outPos = WorldToScreen(Pos, transData);

			if (outPos.z >= 1.0f)
			{
				//draw
			}
			if (outPos.z < 1.0f) continue;

			uintptr_t localEnt = world.GetLocalEntity();

			if (localEnt)
			{
				uintptr_t manvisualState = driver.rpm<uintptr_t>(localEnt + world.dw_manVisualState);

				if (manvisualState)
				{
					Vec3 localPos = driver.rpm<Vec3>(manvisualState + 0x2C);
					int dist = Distance(localPos, Pos);

					if (dist <= itemRenderDistance && dist > 0)
					{
						uintptr_t objectBase = driver.rpm<uintptr_t>(entity + 0xE0);
						uintptr_t cleanNamePtr = driver.rpm<uintptr_t>(objectBase + 0x450);
	

						
						std::string text = getNameFromId(cleanNamePtr);
						if (text.empty()) continue;

						std::stringstream all;
						all << text;
						all << " " << dist;
						DrawString((char*)all.str().c_str(), outPos.x, outPos.y, 240, 240, 260, pFontSmall);
					}
				}
			}
		}
	}
}

bool teleTent = false;
UINT64 airdropContainer = 0;

void IterateSlow(uintptr_t worldptr)
{
	int slowEntityTableSz = driver.rpm<int>(worldptr + 0x1F40 + 0x8); //Slow entity table

	uintptr_t localEnt = world.GetLocalEntity();
	uintptr_t manvisualState = driver.rpm<uintptr_t>(localEnt + world.dw_manVisualState);
	Vec3 localPos = driver.rpm<Vec3>(manvisualState + 0x2C);

	for (size_t i = 0; i < slowEntityTableSz; i++)
	{
		uintptr_t entityTable = driver.rpm<uintptr_t>(worldptr + 0x1F40);

		int check = driver.rpm<int>(entityTable + (i * 0x18));

		if (check == 0) continue;

		uintptr_t entity = driver.rpm<uintptr_t>(entityTable + ((i * 0x18) + 0x8));

		if (entity)
		{
			Vec3 Pos = world.GetObjectVisualState(entity);

			TransData transData = world.GetTransData();

			Vec3 outPos = WorldToScreen(Pos, transData);
			if (outPos.z >= 1.0f)
			{
				if (localEnt)
				{
					if (manvisualState)
					{
						int dist = Distance(localPos, Pos);

						if (dist > 2 && dist <= renderdistance)
						{
							uintptr_t objectBase = driver.rpm<uintptr_t>(entity + 0xE0);
							uintptr_t cleanNamePtr = driver.rpm<uintptr_t>(objectBase + 0x68);

							UINT64 nameId = driver.rpm<UINT64>(cleanNamePtr + 0x10);
							std::string text = getNameFromId(cleanNamePtr);

							if (text.empty()) continue;
							std::stringstream all;
							all << text << " ";

							all << dist; //<< std::endl << objectname;


							DrawString((char*)all.str().c_str(), outPos.x, outPos.y, 240, 240, 250, pFontSmall);
						}
					}
				}
			}
		}
	}
}


bool items = false;

bool gps = false;

Vec3 GpsCoords;

bool initthread = true;

bool enableSpeed = false;
int addr;
bool iterAnimals = false;

bool setInWall = false;
bool setDay = false;
Vec3 TargetPos;

float daylight = 0.0f;

void Hack()
{
	if (initthread)
	{
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)BulletToHeadThread, NULL, NULL, NULL);
		initthread = false;
	}

	if (GetAsyncKeyState(VK_HOME) & 0x8000)
	{
		Sleep(200);
		enableSpeed = !enableSpeed;
		std::cout << enableSpeed << std::endl;

		if (enableSpeed)
		{
			addr = driver.rpm<int>(world.modulebase + 0xD7DA08);
			driver.wpm<int>(addr / 10, world.modulebase + 0xD7DA08);
			//std::cout << addr << std::endl;
		}

		else
		{
			driver.wpm<int>(addr, world.modulebase + 0xD7DA08);
		}
	}


	std::stringstream rend;
	rend << "Render Distance: " << renderdistance << std::endl << "Item Render: " << itemRenderDistance;
	DrawString((char*)rend.str().c_str(), 20, 30, 240, 240, 250, pFontSmall);

	uintptr_t worldptr = driver.rpm<uintptr_t>(world.modulebase + world.dw_world);

	if (GetAsyncKeyState(VK_NUMPAD9) & 0x8000)
	{
		renderdistance += 100;
	}

	if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
	{
		renderdistance -= 100;
	}

	if (GetAsyncKeyState(VK_NUMPAD8) & 0x8000)
	{
		itemRenderDistance += 100;
	}

	if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000)
	{
		itemRenderDistance -= 100;
	}

	if (GetAsyncKeyState(VK_NUMPAD7) & 0x8000)
	{
		Sleep(100);
		gps = !gps;
		GpsCoords.y = 100;
	}

	if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
	{
		GpsCoords.x += 100;
		if (GpsCoords.x >= 16000.f)
		{
			GpsCoords.x = 0;
		}
	}

	if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
	{
		GpsCoords.z += 100;
		if (GpsCoords.z >= 16000.f)
		{
			GpsCoords.z = 0;
		}
	}

	if (GetAsyncKeyState(VK_NUMPAD0) & 0x8000)
	{
		Sleep(100);
		items = !items;
	}

	if (GetAsyncKeyState(VK_DELETE) & 0x8000)
	{
		Sleep(100);
		iterAnimals = !iterAnimals;
	}

	int closestToCenter = 99999;

	if (worldptr)
	{
		uintptr_t localEnt = world.GetLocalEntity();
		uintptr_t manvisualState = driver.rpm<uintptr_t>(localEnt + world.dw_manVisualState);
		Vec3 localPos = driver.rpm<Vec3>(manvisualState + 0x2C);


		if (iterAnimals)
		{
			DrawString((char*)"Slow Animal Table", 20, 105, 240, 240, 250, pFontSmall);
			IterateSlow(worldptr);
		}

		if (items)
		{
			IterateItems(worldptr);
			DrawString((char*)"Items ON", 20, 75, 240, 240, 250, pFontSmall);
		}


		if (setDay)
		{
			DrawString((char*)"Day ON", 20, 150, 240, 240, 250, pFontSmall);

			driver.wpm<float>(2.0f, worldptr + 0x28A8);
		}

		int nearEntityTableSz = driver.rpm<int>(worldptr + world.dw_NearEntityTable + 0x8);
		int farEntityTableSz = driver.rpm<int>(worldptr + world.dw_FarEntityTable + 0x8);


		if (gps)
		{
			TransData transData = world.GetTransData();
			Vec3 gpsOut = WorldToScreen(GpsCoords, transData);
			int distGps = Distance(GpsCoords, localPos);
			std::stringstream gpscoords;

			std::stringstream gpsOn;
			gpsOn << "GPS ON: " << GpsCoords.x << "  " << GpsCoords.z;
			DrawString((char*)gpsOn.str().c_str(), 20, 90, 255, 255, 255, pFontSmall);
			if (gpsOut.z >= 1.0f)
			{

				gpscoords << distGps;
				DrawString((char*)gpscoords.str().c_str(), gpsOut.x, gpsOut.y, 20, 255, 255, pFontSmall);
			}


		}
		for (size_t i = 0; i < nearEntityTableSz; i++)
		{
			TransData transData = world.GetTransData();
			uintptr_t entityTable = driver.rpm<uintptr_t>(worldptr + world.dw_NearEntityTable);
			uintptr_t entity = driver.rpm<uintptr_t>(entityTable + (i * 0x8));
			if (entity)
			{
				uintptr_t networkId = driver.rpm<uintptr_t>(entity + world.dw_networkId);
				if (networkId == 0) continue;

				Vec3 Pos = world.GetVisualState(entity);
				std::stringstream currentPos;
				currentPos << "x: " << localPos.x << "y: " << localPos.z;
				DrawString((char*)currentPos.str().c_str(), 20, 60, 240, 240, 250, pFontSmall);


				Vec3 outPos = WorldToScreen(Pos, transData);

				if (outPos.z >= 1.0f)
				{
					int dist = Distance(localPos, Pos);

					if (dist > 2 && dist <= renderdistance)
					{
						uintptr_t objectBase = driver.rpm<uintptr_t>(entity + 0xE0);
						uintptr_t cleanNamePtr = driver.rpm<uintptr_t>(objectBase + 0x450);

						int size = driver.rpm<int>(cleanNamePtr + 0x8);

						if (size <= 3)
						{
							continue;
						}

						UINT64 nameId = driver.rpm<UINT64>(cleanNamePtr + 0x10);
						std::string text = getNameFromId(cleanNamePtr);
						if (text.empty()) continue;


						std::stringstream all;

						all << text << " ";

						all << dist; 

						if (driver.rpm<unsigned char>(entity + 0xF1) == 1)
						{
							DrawString((char*)all.str().c_str(), outPos.x, outPos.y, 137, 0, 11, pFontSmall);
							continue;
						}
						DrawString((char*)all.str().c_str(), outPos.x, outPos.y, 233, 0, 255, pFontSmall);
					}
				}
			}

			for (size_t i = 0; i < farEntityTableSz; i++)
			{
				TransData transData = world.GetTransData();
				uintptr_t entityTable = driver.rpm<uintptr_t>(worldptr + world.dw_FarEntityTable);
				uintptr_t entity = driver.rpm<uintptr_t>(entityTable + (i * 0x8));
				if (entity)
				{
					int networkId = driver.rpm<int>(entity + world.dw_networkId);

					if (networkId == 0) continue;

					Vec3 Pos = world.GetVisualState(entity);

					TransData transData = world.GetTransData();

					Vec3 outPos = WorldToScreen(Pos, transData);
					if (outPos.z >= 1.0f)
					{
						if (localEnt)
						{
							if (manvisualState)
							{
								int dist = Distance(localPos, Pos);

								if (dist > 2 && dist <= renderdistance)
								{
									uintptr_t objectBase = driver.rpm<uintptr_t>(entity + 0xE0);
									uintptr_t cleanNamePtr = driver.rpm<uintptr_t>(objectBase + 0x450);

									int size = driver.rpm<int>(cleanNamePtr + 0x8);

									if (size <= 3)
									{
										continue;
									}


									UINT64 nameId = driver.rpm<UINT64>(cleanNamePtr + 0x10);
									std::string text = getNameFromId(cleanNamePtr);
									if (text.empty()) continue;

									std::stringstream all;

									all << text << " ";

									all << dist; //<< std::endl << objectname;

									if (driver.rpm<unsigned char>(entity + 0xF1) == 1)
									{
										DrawString((char*)all.str().c_str(), outPos.x, outPos.y, 137, 0, 11, pFontSmall);
										continue;
									}
									DrawString((char*)all.str().c_str(), outPos.x, outPos.y, 233, 0, 255, pFontSmall);
								}
							}
						}
					}
				}
			}
		}
	}
}
//Near Entity Table = 0xE78; // Size = 0xE80
//Further = 0xFC0
//