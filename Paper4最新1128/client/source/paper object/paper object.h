
#pragma once

struct Vector3;
class iex3DObj;

class BasePlayer;

class Paper_obj
{
public:
	Paper_obj();
	virtual ~Paper_obj();

	virtual void Initialize(iex3DObj *model);

	virtual void Update() = 0;

	virtual void Render() = 0;

	void Set_pose(const float angle, const Vector3& pos);

	void Set_user(int number);

	void Set_animframe(int frame);

protected:
	iex3DObj *model; // 3Dモデル

	int number;
	Vector3 position; // 位置
	float angle; // 向きを model に渡すよう
};
