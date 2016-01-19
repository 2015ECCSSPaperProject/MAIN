#pragma once
#include	"../system/Scene.h"

class PaperClient;
class AnimationRippleEx;

class Mouse;

class SceneTitle : public Scene
{
public:
	//初期化・解放
	bool Initialize();
	~SceneTitle();
	//処理
	void Update();
	//描画
	void Render();
	void RenderShadow();

private:
	int FarShadowTimer;
	iexView* view;
	iexMesh* sky;		
	Vector3 LightVec;// Light 
	Vector3 viewPos;
	Vector3 viewTarget;
	enum class STEP
	{
		WAIT,
		CLICK,
		DRAG,
		REND_PAPER,
	};
	STEP step;

	struct
	{
		Vector3 pos;
		iex3DObj *obj;
		bool pointing;
		bool rend;
	}start_button[2];

	enum IMAGE{ BACK, CURSOR, CLICK1, CLICK2, CLICK3, GAME_START, MOUSE, TITLE, ARROW,ICON, ARROW2, MAX };
	iex2DObj* images[MAX];

	Vector2 move_mouse;

	//---------------------------------------------------------------------
	// ImageAnimation
	//---------------------------------------------------------------------
	int arrowPosY;
	int arrowMoveY;

	AnimationRippleEx* titleEx;


	Mouse *mouse;
};