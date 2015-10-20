
#include	"iextreme.h"

//netは一番初めにインクルードしないとエラーが起こる
#include	"../Net/Socket.h"
//#include	"../Net/net_config_loader.h"

#include	"../system/Framework.h"
#include	"../system/Thread.h"

#include	"SceneSelect.h"
#include	"SceneMain.h"

/**********************/
/*	グローバル変数	　*/
/**********************/

//　スレッドを止める為に仮で作った
//static int ThreadEND = false;


using namespace std;

//******************************************************************
//		初期化・解放
//******************************************************************
bool SceneSelect::Initialize()
{

	//	環境設定
	iexLight::SetAmbient(0x808080);
	iexLight::SetFog(800, 3000, 0);

	// Fade処理
	FadeControl::Setting(FadeControl::FADE_IN, 26);

	view = new iexView();
	view->Set(Vector3(0, 30, -60), Vector3(0, 0, 0));

	stage = new iexMesh("DATA/BG/stage_puroto.imo");

	Select = new iex2DObj("DATA/select.png");

	if (!SOCKET_MANAGER->Init())
	{
		MessageBox(0, "サーバーとの接続ができません", null, MB_OK);
		exit(0);
		return false;
	}

	//　名前送る
	SOCKET_MANAGER->SendName();

	// ポスターに必要な情報を初期化
	PosterInit();

	//　ここのステップが STEP::START_OK が出たら
	step = STEP::START_NO;

	m_pThread = 0;

	//	スレッド開始
	//ThreadEND = false;
	m_pThread = new Thread(ThreadFunc, this);
	m_pThread->Run();

	return true;
}

void SceneSelect::PosterInit()
{

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

SceneSelect::~SceneSelect()
{
	//ThreadEND = true;

	delete view;
	delete stage;
	delete Select;

	delete m_pThread;

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

//===================================================================================
//   スレッド
//===================================================================================
void SceneSelect::ThreadFunc(void* pData, bool*isEnd)
{
	SceneSelect* pMulti((SceneSelect*)pData);

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
		switch (pMulti->step)
		{
		case SceneSelect::STEP::START_NO:
			//　セレクト画面の更新
			SOCKET_MANAGER->UpdateTeam(false);
			// レイヤー更新
			SOCKET_MANAGER->UpdateLayer();
			break;
		case SceneSelect::STEP::START_OK:
			//　セレクト画面の更新
			SOCKET_MANAGER->UpdateTeam(true);
			// レイヤー更新
			SOCKET_MANAGER->UpdateLayer();
			break;
		case SceneSelect::STEP::GAME:
			//　セレクト画面の更新
			SOCKET_MANAGER->UpdateTeam(2);
			// レイヤー更新
			SOCKET_MANAGER->UpdateLayer();
			break;
		}
		
	}
}



//******************************************************************
//		処理
//******************************************************************
void SceneSelect::Update()
{


	//ナンバーエフェクト
	Number_Effect::Update();

	//フェード処理
	FadeControl::Update();

	switch (step)
	{
	case STEP::START_NO:
		//　Aボタン押したら
		if (KEY_Get(KEY_ENTER) == 3)
		{
			step = STEP::START_OK;
		}

		break;
	case STEP::START_OK:
	{
		   //　まだ準備できてないので戻ります
		   if (KEY_Get(KEY_ENTER) == 3)
		   {
			   step = STEP::START_NO;
		   }

		   //　追記：同期して
		   // 皆isResdy==2だったらゲーム画面へ！！
		 //  enum { READY = 2 };
		   //int count(0);
		   //int active(0);
		   //for (int i = 0; i < PLAYER_MAX; ++i)
		   //{
			  // if (SOCKET_MANAGER->GetUser(i).com == UserData::ACTIVE_USER)
				 //  ++active;

			  // // isReadyが全員OKになってたら　カウントするよ
			  // if (SOCKET_MANAGER->GetUser(i).isReady == UserData::READY_MUTCH_ALL)
				 //  ++count;
		   //}
		   ////　全員OKだったら飛ぶ
		   //if (active == count)
		   //{
			  // MainFrame->ChangeScene(new SceneMain());
		   //}

		   enum { READY = 2 };
		   if (SOCKET_MANAGER->GetUser(SOCKET_MANAGER->GetID()).isReady == READY)
		   {
		   		//	初期シーン登録
		   		//ThreadEND = true;
				//MainFrame->ChangeScene(new SceneMain());	

			   step = STEP::GAME;

		   }

	}
		break;
	case STEP::GAME:

		//if (KEY_Get(KEY_ENTER, 0) == 3)
		{
			MainFrame->ChangeScene(new SceneMain());
		}

		break;

	default:


		break;
	}

}


//******************************************************************
//		描画
//******************************************************************

void SceneSelect::Render()
{
	view->Activate();

	//　マルチレンダーターゲットでポスターを描画
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		PosterRender(i);
	}
	//バックバッファ
	iexSystem::Device->SetRenderTarget(0, backbuffer);

	view->Clear();

	stage->Render();

	Select->Render(0, 0, 1280, 720, 0, 0, 1280, 720);

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		Text::Draw(800, 40 + (i * 32), 0xffffffff, "Clientaddr[%d]->%ld", i, SOCKET_MANAGER->GetUser(i).com);
		Text::Draw(1000, 40 + (i * 32), 0xffffffff, "name[%d]->%s", i, SOCKET_MANAGER->GetUser(i).name);
		Text::Draw(900, 470 + (i * 32), 0xff00ffff, "isReady[%d]->%d", i, SOCKET_MANAGER->GetUser(i).isReady);
	}

	//ステップ
	switch (step)
	{
	case STEP::GAME:

		Text::Draw(10, 620 , 0xff00ffff, "ENTERで進む");

		break;
	}

	// 皆のポスター
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		posterScreen[i]->Render((i * 160), 580, 512 / 4, 512 / 4, 0, 0, 512, 512);
	}

	//ナンバーエフェクト
	Number_Effect::Render();

	//フェード処理
	FadeControl::Render();

}


//　ポスターの描画　引数はプレイヤーNo
void SceneSelect::PosterRender(int i)
{
	posterScreen[i]->RenderTarget();
	view->Clear(0xffffffff);
	//　ここからポスターのテクスチャーに移る絵を描く

	//　レイヤーの画像
	for (int num = (LAYER_MAX - 1); num >= 0; num--)//■描画順を０が手前に来るように
	{
		if (SOCKET_MANAGER->GetLayer(i).layerdata[num].kind != -1)//　レイヤーが空だったら描画しない
		{
			//　絵を表示　※ポスターの表示場所分ずらしている
			StampPicture[SOCKET_MANAGER->GetLayer(i).layerdata[num].kind][SOCKET_MANAGER->GetLayer(i).layerdata[num].num]
				->Render(SOCKET_MANAGER->GetLayer(i).layerdata[num].x - 256 - (64), SOCKET_MANAGER->GetLayer(i).layerdata[num].y - 256 - (128), 512, 512, 0, 0, 512, 512);
		}
	}

	// ポスターのフレーム ※クライアントの番号によって色を後で分ける
	posterFrame[i]->Render(0, 0, 512, 512, 0, 0, 512, 512);

	//テクスチャーとしてシェーダーへ
	char MapName[64];
	sprintf(MapName, "PosterMap_%d", i);
	shader->SetValue(MapName, posterScreen[i]);

}