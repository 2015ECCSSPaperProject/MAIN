#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_

//********************************************************************************************************************
//
//		クラスの宣言
//
//********************************************************************************************************************
//		サウンドマネージャー
//****************************************************************************************************************

class iex3DSoundIIDX;
class iexSoundIIDX;

//=======================================================================================
//	サウンド実体管理
namespace SoundManager
{
	void Initialize();
	void Release();
	void Update();
	void Set_listener(const Vector3 &pos, const Vector3 &front, const Vector3 &up, const Vector3 &move);
}



//=======================================================================================
//	サウンド基底クラス
class SoundBase
{
protected:
	iex3DSoundIIDX *play_manager;	// iexSound
	int data_no;				// wavデータの種類(番号)


public:
	SoundBase() :play_manager(nullptr){}
	virtual ~SoundBase();
	enum { NOT_FOUND = -1 };

	void Set_listener(const Vector3 &pos, const Vector3 &front, const Vector3 &up, const Vector3 &move);		// リスナー情報

	//===============================================
	//	データ検索
	//===============================================
	virtual int Find_data_no(char *_ID) = 0;
};



//=======================================================================================
//	SE管理クラス
class SE : public SoundBase
{
private:
	int max_count;				// セットしたデータ数

	int Play_in(int no, float volume, bool loop);
	int Play_in(int data_num, const Vector3 &pos, const Vector3 &front, bool loop);

public:

	//===============================================
	//	音データ
	//===============================================
	struct	DATA{
		char *id;				// 呼び出す際に使用する識別コード
		char *file_name;		// ファイルパス
		int same_play_max;		// 同一の音を複数再生する最大値
		float volume;			// ボリューム
		bool b3D;				// 3Dフラグ
		int start_num;			// 1つのwavデータあたり start_num 〜 start_num + same_play_max 分のデータを保持
	};


	//===============================================
	//	初期化と解放
	//===============================================
	SE() :SoundBase(){}
	void Initialize();

	//===============================================
	//	更		新
	//===============================================
	void Update();


	//===============================================
	//	処		理
	//===============================================
	int Play(char *_ID, bool loop = false);			// 簡易版
	int Play(char *_ID, const Vector3 &pos, const Vector3 &front = Vector3(0, 0, -1), bool loop = false);	// 3D設定版
	void Stop(int no);								// Playで返ってきた数値を入れる
	void Stop_all();								// 全部止める
	bool isPlay(char *ID);
	bool isPlay(int no);
	void Set_data(int no, const Vector3 &pos, const Vector3 &front = Vector3(0, 0, -1), const Vector3 &move = Vector3(0, 0, 0));

	//===============================================
	//	データ検索
	//===============================================
	int Find_data_no(char *_ID);
};


//=======================================================================================
//	BGM管理クラス
class BGM : public SoundBase
{
private:
	int Play_in(int no, float volume, bool loop);

public:
	enum F_TYPE{ FADE_NONE = 0, FADE_IN, FADE_OUT };
	void None(int no);
	void In(int no);
	void Out(int no);
	void(BGM::*Fade_mode_funk[3])(int);

	//===============================================
	//	音データ
	//===============================================
	struct	DATA{
		char *id;					// 呼び出す際に使用する識別コード
		char *file_name;			// ファイルパス
		float max_vol;				// 最大音量
		float fade_speed;			// フェードスピード
		F_TYPE fade_type;			// 関数が分岐する
		float volume;				// ボリューム
	};


	//===============================================
	//	初期化と解放
	//===============================================
	BGM() :SoundBase(){}
	void Initialize();

	//===============================================
	//	更		新
	//===============================================
	void Update();


	//===============================================
	//	処		理
	//===============================================
	int Play(char *_ID, bool loop = true);				// 簡易版
	int Play(char *_ID, float volume, bool loop);		// フル設定版
	int Fade_in(char *_ID, int speed, bool loop = true);// フェードイン

	void Stop(char *_ID);								// 停止
	void Fade_out(char *_ID, int speed);				// フェードアウト
	void Stop_all();									// 全部止める
	void Pause(char *_ID);								// ポーズ
	bool isPlay(char *_ID);

	void Set_speed(char *_ID, float speed);				// 再生速度変更

	//===============================================
	//	データ検索
	//===============================================
	int Find_data_no(char *_ID);
};


//=======================================================================================
//	イベントでいろいろ
class EventBGM
{
public:
	enum MODE
	{
		NONE, 	// 何もしない
		START, 	// ゲーム始まったら呼び出す
		KOUHAN, // 制限時間 1 / 4　ぐらいで呼び出す
		END,	// ゲーム終わったら呼び出す
		MAX
	};

private:
	char mainBGM[64];
	int step;
	int frame;

	MODE mode;
	bool isKouhan;
	void None();
	void Start();
	void Kouhan();
	void End();
	void(EventBGM::*Mode_funk[MODE::MAX])();

public:

	// 実体取得
	static EventBGM *getInstance()
	{
		static EventBGM i;
		return &i;
	}

	// 初期化
	EventBGM();
	void Initialize(char *mainBGMname);

	// 更新
	void Update();

	void Set_mode(MODE m)
	{
		mode = m;
	}
	MODE Get_mode(){ return mode; }
};
#define event_bgm ( EventBGM::getInstance() )

//===============================================
//	実体
//===============================================
extern SE *se;
extern BGM *bgm;


#endif