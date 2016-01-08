
#pragma once

//#include <vector>
//#include "iextreme.h"

struct Vector3;
class iex3DObj;

class BasePlayer;
class Paper_obj;

enum KIND_PAPER_OBJECT;

class Paper_obj_mng
{
public:
	Paper_obj_mng();
	~Paper_obj_mng();

	void Initialize();
	void Release();
	void Update();
	void Render(BasePlayer *player, iexShader *shader = nullptr, char *name = '\0');

	//**************************************************
	/// プレイヤーからのアクション
	//**************************************************

	// 破れるオブジェクトの番号
	int Can_do(BasePlayer *player);

	void Can_dist(const Vector3 &pos, float dist, int out[]); // 範囲内全部返す

	// Can_doの後で呼ぶ
	bool Can_rend(int index);

	// 破る
	void Rend_poster(int index);

	//**************************************************
	/// Get Set
	//**************************************************
	
	int Get_numof();
		
	void Set_user(int index, int number);

	void Set_animframe(int index, int frame);

	const Vector3& Get_pos(int index);

	KIND_PAPER_OBJECT Get_kind( int index );

	//**************************************************

	unsigned int Get_receive_data_size();

	void Set_receive_data( char *in );

protected:
	// クローン関数使う用
	iex3DObj *original_flyer;
	iexMesh *original_model;
	int number_of_objects;

	std::vector<Paper_obj*> obj_array;

	void Load();

	void Load_flyer();
	template<class POSTERCLASS>void Load_poster_tmp( char *filename, iexMesh *model );

	// 矢印
	iex2DObj *mark;
};

extern Paper_obj_mng *paper_obj_mng;
