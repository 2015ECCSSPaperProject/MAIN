#include	"iextreme.h"

//netは一番初めにインクルードしないとエラーが起こる
#include	"../Net/Socket.h"
//#include	"../Net/net_config_loader.h"

#include	"../system/Framework.h"
#include	"../system/Thread.h"

#include	"SceneMain.h"
#include	"../Player/BasePlayer.h"
#include	"../Player/MyPlayer.h"
#include	"../Player/NetPlayer.h"
#include	"../sound/SoundManager.h"

#include	"../Scene/SceneResult.h"
//#include	"../data/LimitedData.h"
#include	"../ui/UI.h"

//net
//#include "Network/TCPServerSocket.h"
//#include "Network/net_config_loader.h"


//#include <process.h>
//#include <thread>
//#include <vector>
//#include <queue>
//#include <mutex>
//#include <condition_variable>
//#include <atomic>
//#include <WS2tcpip.h>

#include	"../bench/Bench_mark.h"
extern Bench_mark bench;

// アラミタマ
//iex3DObj *set;
//iex3DObj *set_die;
#include	"../Player/PlayerManager.h"


#include "../camera/Camera.h"
#include "../stage/Stage.h"

#include "../poster/Poster_manager.h"
#include "../score/Score.h"
#include "../timer/Timer.h"

using namespace std;

static int FLAME = 0;

//******************************************************************
//		初期化・解放
//******************************************************************
bool SceneMain::Initialize()
{

	//	環境設定
	iexLight::SetAmbient(0x808080);
	iexLight::SetFog(800, 3000, 0);

	// Fade処理
	FadeControl::Setting(FadeControl::FADE_IN, 26);

	//view = new iexView();
	//view->Set(Vector3(0, 10, -60), Vector3(0, 10, 0));
	camera = new Camera;


	//stage = new iexMesh2("DATA/BG/stage_puroto.imo");
	stage = new Stage;
	stage->Initialize();

	sky = new iexMesh("DATA/Skydome/Skydome.IMO");
	sky->SetScale(4.0f);
	sky->SetPos(0, -100, 0);
	sky->Update();

	//set = new iex3DObj("DATA/CHR/player/run.IEM");
	//set_die = new iex3DObj("DATA/CHR/player/die.IEM");

	//■■■　相手と自分で分ける
	//for (int i = 0; i <PLAYER_MAX; ++i)
	//{
	//	if (i == SOCKET_MANAGER->GetID()){
	//		player[i] = new MyPlayer();
	//		camera->Initialize(player[i]);	// カメラ設定
	//	}
	//	else
	//		player[i] = new NetPlayer();

	//	player[i]->Initialize(set, set_die);
	//}

	player_mng = new PlayerManager;
	player_mng->Initialize();

	poster_mng = new Poster_manager;
	poster_mng->Initialize(score_mng);

	// タイマー
	timer = new Timer;

	// UI
	ui = new UI;
	ui->Initialize(player_mng->Get_player(SOCKET_MANAGER->GetID()));

	event_bgm->Initialize("フライハイ");
	event_bgm->Set_mode(EventBGM::MODE::START);

	// 開始フラグを送る
	m_pThread = new Thread(ThreadFunc, this);

	//	同期をとる
	SOCKET_MANAGER->InitGame();//ここで

	//	ネットワークを開始する
	m_pThread->Run();

	// モード関数初期化
	mode = MODE::START;
	Mode_funk[0] = &SceneMain::Start;
	Mode_funk[1] = &SceneMain::Main;
	Mode_funk[2] = &SceneMain::End;

	FLAME = 0;

	return true;


}

SceneMain::~SceneMain()
{
	delete camera;
	//delete view;
	delete stage;
	delete sky;
	delete m_pThread;//　なんかスレッド消さないとエラー起こる

	SAFE_DELETE(player_mng);

	SAFE_DELETE(poster_mng);
	SAFE_DELETE(ui);
	SAFE_DELETE(timer);
}

//===================================================================================
//   スレッド
//===================================================================================
void SceneMain::ThreadFunc(void* pData, bool*isEnd)
{
	SceneMain* pMulti((SceneMain*)pData);

	/*フレームワーク*/
	DWORD dwFrameTime(clock());
	while (true)
	{
		/*スレッド終了*/
		if (*isEnd)
			return;

		/*フレーム制限*/
		DWORD CurrentTime = clock() * 10;
		if (CurrentTime < dwFrameTime + 167)
			continue;

		/*経過時間*/
		DWORD	dTime = CurrentTime - dwFrameTime;
		if (dTime > 2000) dwFrameTime = CurrentTime;

		dwFrameTime += 167;

		/*ネットワーク更新*/
		FLAME++;
		//if (FLAME < 60*5)
		{
			SOCKET_MANAGER->UpdateUser();
			SOCKET_MANAGER->UpdateStage();
			SOCKET_MANAGER->UpdateScore();
		}

	}
}

//******************************************************************
//		処理
//******************************************************************
void SceneMain::Update()
{
	// スレッド→ここに移動することによってscene跨ぎのバグが治る
	if (KEY(KEY_ENTER) == 3)
	{
	}

	//フェード処理
	FadeControl::Update();
	event_bgm->Update();
	ui->Update();
	(this->*Mode_funk[mode])();
}

void SceneMain::Start()
{
	if (1)mode = MODE::MAIN;
}
void SceneMain::Main()
{
	//　プレイヤー
	player_mng->Update();

	// カメラ
	camera->Update();

	//ナンバーエフェクト
	Number_Effect::Update();

	//　仮の処理
	static float angle = 0;
	angle += 0.0007f;
	sky->SetAngle(angle);
	sky->Update();

	if (timer->Get_limit_time() == 0)
	{
		event_bgm->Set_mode(EventBGM::MODE::END);
		ui->Change_mode(SceneMain::MODE::END);
		mode = MODE::END;
		m_pThread->End();
	}
}
void SceneMain::End()
{
	if (event_bgm->Get_mode() == EventBGM::MODE::NONE && FadeControl::isFadeOut_W)
	{
		se->Stop_all();
		// シーン登録
		MainFrame->ChangeScene(new SceneResult());
	}
}



//******************************************************************
//		描画
//******************************************************************

void SceneMain::Render()
{
	camera->Activate();
	camera->Clear();

	stage->Render();
	sky->Render();

	//　プレイヤー
	//for (int i = 0; i < PLAYER_MAX; ++i)
	//{
	//	player[i]->Render();
	//	Text::Draw(1100, 20 + (i * 32), 0xff00ffff, "pos.x->%.2f", player[i]->Get_pos().x);

	//	Text::Draw(950, 20 + (i * 32), 0xff00ffff, "名前：%s", SOCKET_MANAGER->GetUser(i).name);
	//}
	player_mng->Render();

	poster_mng->Render();

	Text::Draw(32, 320, 0xff00ffff, "受信時間%.2f", bench.Get_time());

	//マウスの場所
	//Text::Draw(10, 60, 0xff000000, "マウスのXの動き%.2f", player_mng->Get_player(SOCKET_MANAGER->GetID())->m_controlDesc.mouseX);
	//Text::Draw(10, 80, 0xff000000, "マウスのYの動き%.2f", player_mng->Get_player(SOCKET_MANAGER->GetID())->m_controlDesc.mouseY);

	//ナンバーエフェクト
	Number_Effect::Render();

	// UI
	ui->Render();

	// 退魔ー(UIで描画)
	//timer->Render();

	//フェード処理
	FadeControl::Render();

}