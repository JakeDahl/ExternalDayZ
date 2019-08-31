#pragma once
#include "Utils.hpp"
#include <DriverController.h>

#pragma comment(lib, "DriverController.lib")
using namespace DriverControl;

DriverController driver("DayZ_x64.exe");

class World
{
public:
	World()
	{
		this->modulebase = driver.GetModuleBase("DayZ_x64.exe");
		std::cout << std::hex << this->modulebase << std::endl << std::endl;
	}

	uintptr_t GetLocalEntity();
	TransData GetTransData();
	Vec3 GetVisualState(uintptr_t entity);
	Vec3 GetObjectVisualState(uintptr_t entity);

	uintptr_t modulebase = NULL;
	uintptr_t dw_world = 0xDEA710;
	uintptr_t dw_network_manager = 0xD96A70;
	uintptr_t dw_playerOn = 0x2890;
	uintptr_t dw_camera = 0x1B8;
	uintptr_t dw_manVisualState = 0x88;
	uintptr_t dw_objectVisualState = 0x130;
	uintptr_t dw_networkId = 0x588;
	uintptr_t dw_NearEntityTable = 0xE90;
	uintptr_t dw_FarEntityTable = 0xFD8;
	uintptr_t dw_bulletTable = 0xD48;
	uintptr_t dw_ItemsOnGround = 0x1F90;
	//uintptr_t dw_ItemsOnGround = 0x1f78;
	//uintptr_t dw_itemName = 0x618; //dereference as char*
private:
	//Near Entity Table = 0xE78; // Size = 0xE80
//Further = 0xFC0
//
};


uintptr_t World::GetLocalEntity()
{
	uintptr_t dwWorld = driver.rpm<uintptr_t>(this->modulebase + this->dw_world);
	if (dwWorld)
	{
		uintptr_t playerOn = driver.rpm<uintptr_t>(dwWorld + this->dw_playerOn);
		if (playerOn)
		{
			uintptr_t entity = driver.rpm<uintptr_t>(playerOn + 0x8);
			if (entity)
			{
				return entity;
			}
		}
	}
	return NULL;
}

TransData World::GetTransData()
{
	uintptr_t dwWorld = driver.rpm<uintptr_t>(this->modulebase + this->dw_world);
	if (dwWorld)
	{
		uintptr_t camera = driver.rpm<uintptr_t>(dwWorld + this->dw_camera);

		if (camera)
		{
			TransData transData = driver.rpm<TransData>(camera);
			return transData;
		}
	}

	TransData data;
	return data;
}

Vec3 World::GetVisualState(uintptr_t entity)
{
	if (entity)
	{
		uintptr_t renderVisualState = driver.rpm<uintptr_t>(entity + this->dw_objectVisualState);

		if (renderVisualState)
		{
			Vec3 pos = driver.rpm<Vec3>(renderVisualState + 0x2C);
			return pos;
		}
	}

	return Vec3(-1, -1, -1);
}

Vec3 World::GetObjectVisualState(uintptr_t entity)
{
	if (entity)
	{
		uintptr_t renderVisualState = driver.rpm<uintptr_t>(entity + this->dw_objectVisualState);

		if (renderVisualState)
		{
			Vec3 pos = driver.rpm<Vec3>(renderVisualState + 0x2C);
			return pos;
		}
	}

	return Vec3(-1, -1, -1);
}