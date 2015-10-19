#pragma once
/*	自分のプレイヤー	*/

/*前方宣言*/
//class BasePlayer;

//********************************************************************************************************************
//		自分
//****************************************************************************************************************
class	MyPlayer : public	BasePlayer
{
public:
	//===============================================
	//	初期化と解放
	//===============================================
	MyPlayer();
	~MyPlayer();
	void Initialize(iex3DObj *obj, iex3DObj *die);
	void Release();


	//===============================================
	//	更新と描画
	//===============================================
	void Update();
	void Render();

	void Mouse_Update();//	マウスのポジション

	//===============================================
	//	ゲッター,セッター
	//===============================================



};