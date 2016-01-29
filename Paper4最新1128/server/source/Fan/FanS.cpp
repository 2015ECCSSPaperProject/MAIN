#include "iextreme.h"
#include "../fstream/fstream_paper.h"
#include "FanS.h"

void FanMng::Load(char *filename)
{
	std::ifstream infs(filename);

	// 個数
	infs >> num_fan;
	fan_pos = new Vector3[num_fan];

	// 位置とか
	for (int i = 0; i < num_fan; i++)
	{
		infs >> fan_pos[i];
	}
	infs.close();
}

FanMng::FanMng()
{
	Load("DATA/Fan/set.txt");
}
FanMng::~FanMng()
{
	delete fan_pos;
}

int FanMng::CheckFan(float dist, const Vector3 &pos)
{
	for (int i = 0; i < num_fan; i++)
	{
		// xとzでとる(円柱)
		if ((pos.x - fan_pos[i].x)*(pos.x - fan_pos[i].x) + (pos.z - fan_pos[i].z)*(pos.z - fan_pos[i].z) < dist*dist)
		{
			return i;
		}
	}
	return -1;
}

bool FanMng::CheckHeight(int no, float dist, float pos_y)
{
	assert(no != -1);
	// xとzでとる(円柱)
	if (pos_y - fan_pos[no].y > dist)
	{
		return false;
	}
	return true;
}