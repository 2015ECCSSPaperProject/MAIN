#pragma once
#include	"../system/Scene.h"

class PaperClient;

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

private:
	iexView* view;
	iex2DObj* title;


};
