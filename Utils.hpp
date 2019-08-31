#include <Windows.h>


class Vec2
{
public:
	float x, y;
	Vec2() { x = y = 0.0f; };
	void Zero() { x = y = 0.0f; };
	void NineNull() { x = y = 999.9f; };
	friend Vec2 operator - (Vec2 a, Vec2 b)
	{
		Vec2 temp;
		temp.x = a.x - b.x;
		temp.y = a.y - b.y;
		return temp;
	};
	friend Vec2 operator + (Vec2 a, Vec2 b)
	{
		Vec2 temp;
		temp.x = a.x + b.x;
		temp.y = a.y + b.y;
		return temp;
	};
	friend void operator += (Vec2& a, Vec2 b)
	{
		a.x += b.x;
		a.y += b.y;
	};
	friend void operator -= (Vec2& a, Vec2 b)
	{
		a.x -= b.x;
		a.y -= b.y;
	};
	friend Vec2 operator / (Vec2 a, float b)
	{
		Vec2 temp;
		temp.x = a.x / b;
		temp.y = a.y / b;
		return temp;
	};
};

class Vec3
{
public:
	float x, y, z;
	Vec3() { x = y = z = 0.0f; };
	Vec3(float X, float Y, float Z) { x = X; y = Y; z = Z; };
	void Zero() { x = y = z = 0.0f; };
	void NineNull() { x = y = z = 999.9f; };
	friend Vec3 operator - (Vec3 a, Vec3 b)
	{
		Vec3 temp;
		temp.x = a.x - b.x;
		temp.y = a.y - b.y;
		temp.z = a.z - b.z;
		return temp;
	};
	friend Vec3 operator + (Vec3 a, Vec3 b)
	{
		Vec3 temp;
		temp.x = a.x + b.x;
		temp.y = a.y + b.y;
		temp.z = a.z + b.z;
		return temp;
	};
	friend void operator += (Vec3& a, Vec3 b)
	{
		a.x += b.x;
		a.y += b.y;
		a.z += b.z;
	};
	friend void operator -= (Vec3& a, Vec3 b)
	{
		a.x -= b.x;
		a.y -= b.y;
		a.z -= b.z;
	};
	friend Vec3 operator * (Vec3 a, float b)
	{
		Vec3 temp;
		temp.x = a.x * b;
		temp.y = a.y * b;
		temp.z = a.z * b;
		return temp;
	};
};

struct TransData
{
	char unknown0[8]; //0x0000
	Vec3 InvView_right; //0x0004
	Vec3 InvView_up; //0x0010  
	Vec3 InvView_forward; //0x001C  
	Vec3 InvView_Translation; //0x0028  

	Vec2 pad; //0x0034   (x = 1,y = -1)


	Vec3 ViewPortMatrix1; //0x003C  (x= viewPort.Width/2)
	Vec3 ViewPortMatrix2; //0x0048 -(y= viewPort.Height/2)

	Vec3 ViewPortMatrix3;
	//0x0054(x=viewPort.X + CenterScreenX,y=viewPort.Y + CenterScreenY,z=viewPort.MinZ)

	Vec3 ViewPortMatrix_unk; //not sure

	Vec3 ProjO1; //proj1.x  0x006C
	Vec3 ProjO2; //proj2.y  0x0078
	Vec3 ProjO3; //proj3.z  0x0084     =1.0001
	Vec3 ProjO4; //proj4.z  0x0090

	Vec3 Proj1; //proj1.x  0x009C
	Vec3 Proj2; //proj2.y  0x00A8
	Vec3 Proj3; //proj3.z  0x00B4     = 1.0
	Vec3 Proj4; //proj4.z  0x00C0

	Vec3 ProjD1; //   0x00CC     x = 1/Proj._11
	Vec3 ProjD2; //   0x00D8     y = 1/Proj._22
	Vec3 ProjD3; //   0x00E4     z=  1
	Vec3 ProjD4; //   0x00F0

	Vec2 pad1; //.x bigass float  0x00FC

	Vec3 View_right; //0x0104    not the view
	Vec3 View_up; //0x0110  
	Vec3 View_forward; //0x011C  
	Vec3 View_Translation; //0x0128
};

float Dot(Vec3 left, Vec3 right)
{
	return (left.x * right.x) + (left.y * right.y) + (left.z * right.z);
}

int Distance(Vec3 first, Vec3 second)
{
	return (int)sqrtf(((second.x - first.x) * (second.x - first.x)) + ((second.y - first.y) * (second.y - first.y)) + ((second.z - first.z) * (second.z - first.z)));
}

Vec3 WorldToScreen(Vec3 pos, TransData transData)
{
	Vec3 temp;
	temp.x = pos.x - transData.InvView_Translation.x;
	temp.y = pos.y - transData.InvView_Translation.y;
	temp.z = pos.z - transData.InvView_Translation.z;

	float x = Dot(temp, transData.InvView_right);
	float y = Dot(temp, transData.InvView_up);
	float z = Dot(temp, transData.InvView_forward);

	Vec3 ToReturn;
	ToReturn.x = transData.ViewPortMatrix3.x * (1 + ((x / transData.ProjD1.x) / z));
	ToReturn.y = transData.ViewPortMatrix3.y * (1 - ((y / transData.ProjD2.y) / z));
	ToReturn.z = z;
	return ToReturn;
}

Vec3 Cross(Vec3 a, Vec3 b)
{
	Vec3 product;
	product.x = (a.y * b.z) - (a.z * b.y);
	product.y = (a.z * b.x) - (a.x * b.z);
	product.z = (a.x * b.y) - (a.y * b.x);
	return product;
}
