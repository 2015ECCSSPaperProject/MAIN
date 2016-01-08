
#pragma once

struct Vector3;
class iex3DObj;
class iexMesh;

class BasePlayer;

enum KIND_PAPER_OBJECT;

class Paper_obj
{
public:
	Paper_obj();
	virtual ~Paper_obj();

	virtual void Update() = 0;

	virtual void Render(iexShader *shader = nullptr, char *name = '\0') = 0;

	void Set_pose(const float angle, const Vector3& pos);

	void Set_user(int number);

	void Set_animframe( int frame )
	{};

	const Vector3& Get_pos();

	virtual KIND_PAPER_OBJECT Get_kind() = 0;

	void Render_mark(iex2DObj *obj, bool target);	// 上の矢印

	//**************************************************

	virtual unsigned int Get_receive_data_size() = 0;

	virtual void Set_receive_data( char *in ) = 0;

protected:
	int number;
	Vector3 position; // 位置
	float angle; // 向きを model に渡すよう
};

class Paper_obj_Mesh : public Paper_obj
{
public:
	Paper_obj_Mesh();
	~Paper_obj_Mesh();
	virtual void Initialize( iexMesh *model );

	void Set_pose( const float angle, const Vector3& pos );

protected:
	iexMesh *model; // 3Dモデル
};

class Paper_obj_3DObj : public Paper_obj
{
public:
	Paper_obj_3DObj();
	~Paper_obj_3DObj();
	virtual void Initialize( iex3DObj *model );

	void Set_pose( const float angle, const Vector3& pos );

	void Set_animframe( int frame );

protected:
	iex3DObj *model; // 3Dモデル
};
