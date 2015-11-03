#include	"../system/Framework.h"
//net
#include	"../Net/PaperServer.h"
#include	"SceneMainServer.h"

#include	"../system/Thread.h"
#include	"../Player/BasePlayer.h"
#include	"../Player/NetPlayer.h"
#include	"../Player/Kohai.h"

#include	"../stage/Stage.h"

using namespace std;

// アラミタマ
iex3DObj *set;

#include "../poster/Poster_manager.h"
#include "../score/Score.h"
#include "../timer/Timer.h"

//******************************************************************
//		初期化・解放
//******************************************************************
SceneMainServer::SceneMainServer()
{
	//	環境設定
	iexLight::SetAmbient(0x808080);
	iexLight::SetFog( 800, 3000, 0 );

	// Fade処理
	FadeControl::Setting(FadeControl::FADE_IN, 26);

	view = new iexView();
	view->Set(Vector3(0, 600, -60), Vector3(0, 0, 0));
	view->Activate();

	//stage = new iexMesh2("DATA/BG/stage_puroto.imo");
	stage = new Stage;
	stage->Initialize();

	set = new iex3DObj("DATA/CHR/player/run.IEM");

	//　プレイヤー初期化
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		player[i] = new NetPlayer(i);
		player[i]->Initialize(set);
		kohai[i] = new Kohai(i);
		kohai[i]->Initialize(set);
	}
	
	score = new Score;
	poster_mng = new Poster_manager;
	poster_mng->Initialize(score);

	// ポスター
	PosterInit();

	//	スレッド開始
	m_pThread = 0;
	isRun = true;

	m_pThread = new Thread(ThreadFunc, this);
	m_pThread->Run();

	timer = new Timer;
}

void SceneMainServer::PosterInit()
{
	//　↓の種類に名前を付ける
	enum KIND_NAME{
		OBJECT = 0, FOOD = 1, BACK = 2
	};

	//　イラスト初期化＆スタンプ手帳初期化
	for (int i = 0; i < KIND_MAX; i++)
	{
		for (int no = 0; no < NO_MAX; no++)
		{
			StampPicture[i][no] = NULL;
		}
	}

	//イラストはここで入れていきます。
	StampPicture[KIND_NAME::OBJECT][0] = new iex2DObj("DATA/makePoster/stamp/yen.png");
	StampPicture[KIND_NAME::OBJECT][1] = new iex2DObj("DATA/makePoster/stamp/coffee.png");
	StampPicture[KIND_NAME::OBJECT][2] = new iex2DObj("DATA/makePoster/stamp/gentei.png");
	StampPicture[KIND_NAME::OBJECT][3] = new iex2DObj("DATA/makePoster/stamp/hamburger.png");
	StampPicture[KIND_NAME::FOOD][0] = new iex2DObj("DATA/makePoster/stamp/aniki.png");
	StampPicture[KIND_NAME::FOOD][1] = new iex2DObj("DATA/makePoster/stamp/歪みねぇな.png");
	StampPicture[KIND_NAME::FOOD][2] = new iex2DObj("DATA/makePoster/stamp/蟹.png");
	StampPicture[KIND_NAME::FOOD][3] = new iex2DObj("DATA/makePoster/stamp/だらしねぇな.png");
	StampPicture[KIND_NAME::BACK][0] = new iex2DObj("DATA/makePoster/stamp/back/not.png");
	StampPicture[KIND_NAME::BACK][1] = new iex2DObj("DATA/makePoster/stamp/back/tehutehu.png");
	StampPicture[KIND_NAME::BACK][2] = new iex2DObj("DATA/makePoster/stamp/back/block.png");
	StampPicture[KIND_NAME::BACK][3] = new iex2DObj("DATA/makePoster/stamp/back/midori.png");
	StampPicture[KIND_NAME::BACK][4] = new iex2DObj("DATA/makePoster/stamp/back/blue.png");
	StampPicture[KIND_NAME::BACK][5] = new iex2DObj("DATA/makePoster/stamp/back/ゲイパレス.png");

	//スクリーン
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		posterScreen[i] = new iex2DObj(512, 512, IEX2D_RENDERTARGET);

		//　色つきフレーム
		char fileName[128];
		sprintf(fileName, "DATA/makePoster/frame/Posterframe%d.png", i);
		posterFrame[i] = new iex2DObj(fileName);
	}

	iexSystem::Device->GetRenderTarget(0, &backbuffer);//★描画先


};

bool SceneMainServer::Initialize()
{

	////	環境設定
	//iexLight::SetAmbient(0x808080);
	//iexLight::SetFog( 800, 3000, 0 );

	//// Fade処理
	//FadeControl::Setting(FadeControl::FADE_IN, 26);

	//view = new iexView();
	//view->Set(Vector3(0, 100, -60), Vector3(0, 0, 0));
	//view->Activate();

	//stage = new iexMesh2("DATA/BG/stage_puroto.imo");

	////	スレッド開始
	//m_pThread = 0;
	//isRun = true;

	//m_pThread = new Thread(ThreadFunc, this);
	//m_pThread->Run();


	return true;

}


//===================================================================================
//    スレッド
//===================================================================================
void SceneMainServer::ThreadFunc(void* pData, bool*isEnd)
{
	SceneMainServer* pMulti((SceneMainServer*)pData);

	//　サーバー初期化
	ServerManager::Init();


	/*フレームワーク*/
	DWORD dwFrameTime(clock());
	while (pMulti->isRun)
	{
		/*スレッド終了*/
		if (*isEnd)
			return;

		///*フレーム制限*/
		//DWORD CurrentTime = clock() * 10;
		//if (CurrentTime < dwFrameTime + 167) // 0.0167f (1フレーム)
		//	continue;

		///*経過時間*/
		//DWORD	dTime = CurrentTime - dwFrameTime;
		//if (dTime > 2000) dwFrameTime = CurrentTime;

		//dwFrameTime += 167;

		/*ネットワーク更新*/
		ServerManager::Update();
	}

	//ServerManager::Release();
}

SceneMainServer::~SceneMainServer()
{
	SAFE_DELETE(timer);

	delete view;
	delete stage;
	
	//isRun = false;
	//　今ここで消す
	ServerManager::Release();
	delete m_pThread;

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		SAFE_DELETE(player[i]);
		SAFE_DELETE(kohai[i]);
	}

	delete set;

	SAFE_DELETE(poster_mng);
	SAFE_DELETE(score);


	// スタンプ
	for (int i = 0; i < KIND_MAX; i++)
	{
		for (int no = 0; no < NO_MAX; no++)
		{
			SAFE_DELETE(StampPicture[i][no]);
		}
	}

	// スクリーン
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		SAFE_DELETE(posterScreen[i]);
		SAFE_DELETE(posterFrame[i]);
	}

}


//******************************************************************
//		処理
//******************************************************************
void SceneMainServer::Update()
{
	// タイマー更新
	timer->Check();

	//ナンバーエフェクト
	Number_Effect::Update();

	//フェード処理
	FadeControl::Update();

	// サーバー
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		player[i]->Update();
		kohai[i]->Update();
	}
	
	poster_mng->Update();
}



//******************************************************************
//		描画
//******************************************************************

void SceneMainServer::Render()
{
	//　マルチレンダーターゲットでポスターを描画
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		PosterRender(i);
	}

	//バックバッファ
	iexSystem::Device->SetRenderTarget(0, backbuffer);

	view->Clear();

	stage->Render();

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		player[i]->Render();
		Text::Draw(1000, 50 + (i * 40), 0xffff00ff, "pos.x->%.2f", player[i]->Get_pos().x);
		kohai[i]->Render();
	}

	poster_mng->Render();

	// 皆のポスター
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		posterScreen[i]->Render((i * 160), 580, 512 / 4, 512 / 4, 0, 0, 512, 512);
	}

	// サーバー描画
	ServerManager::Render();

	//ナンバーエフェクト
	Number_Effect::Render();

	//フェード処理
	FadeControl::Render();

	int m, s, ms;
	timer->Get_second_limit(&m, &s, &ms);
	Text::Draw(500, 50, 0xffff00ff, "TIME : %d : %02d : %03d", m, s, ms);
}


//　ポスターの描画　引数はプレイヤーNo
void SceneMainServer::PosterRender(int i)
{
	posterScreen[i]->RenderTarget();
	view->Clear(0xffffffff);
	//　ここからポスターのテクスチャーに移る絵を描く

	//　レイヤーの画像
	for (int num = (LAYER_MAX - 1); num >= 0; num--)//■描画順を０が手前に来るように
	{
		if (ServerManager::GetLayer(i).layerdata[num].kind != -1)//　レイヤーが空だったら描画しない
		{
			//　絵を表示　※ポスターの表示場所分ずらしている
			StampPicture[ServerManager::GetLayer(i).layerdata[num].kind][ServerManager::GetLayer(i).layerdata[num].num]
				->Render(ServerManager::GetLayer(i).layerdata[num].x - 256 - (64), ServerManager::GetLayer(i).layerdata[num].y - 256 - (128), 512, 512, 0, 0, 512, 512);
		}
	}

	// ポスターのフレーム ※クライアントの番号によって色を後で分ける
	posterFrame[i]->Render(0, 0, 512, 512, 0, 0, 512, 512);

}