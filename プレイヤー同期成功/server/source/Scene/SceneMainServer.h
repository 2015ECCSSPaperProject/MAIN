#ifndef _SCENEMAIN_H_
#define _SCENEMAIN_H_

#include	"../system/Scene.h"

class ShadowMap;
class Stage;
class Thread;

class SceneMainServer : public Scene
{
public:
	//初期化・解放
	SceneMainServer();
	bool Initialize();

	~SceneMainServer();
	//処理
	void Update();
	//描画
	void Render();
	
private:
	iexView* view;
	Stage* stage;


	//	通信用
	bool isRun;
	Thread* m_pThread;
	//---------------------------------------------------------------------
	//   thread
	//---------------------------------------------------------------------
	static void ThreadFunc(void* pData, bool*);

};

#endif // !_SCENEMAIN_H_