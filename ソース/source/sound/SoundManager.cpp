
#include	"iextreme.h"
#include	"../IEX/IEX_AudioIIDX.h"
#include	"SoundManager.h"



//********************************************************************************************************
//
//									実体管理
//
//********************************************************************************************************
void SoundManager::Initialize()
{
	se = new SE, se->Initialize();
	bgm = new BGM, bgm->Initialize();
}

void SoundManager::Release()
{
	se->Release(), delete se;
	bgm->Release(), delete bgm;
}

void SoundManager::Update()
{
	se->Update();
	bgm->Update();
}



//********************************************************************************************************
//
//									S	E
//
//********************************************************************************************************

//*********************************************************************************************
//		パラメータの設定
//*********************************************************************************************
//	SEデータ(textで読み込むのも良いかもしれない)
SE::DATA	all_se_data[] = 
{
	{ "歩行", "DATA/Sound/SE/paper-move.wav", 1, 1.0f },
	{ "ポスター貼る", "DATA/Sound/SE/paper-haru.wav", 1, 1.0f },
	{ "ポスター破る", "DATA/Sound/SE/紙を破る音.wav", 1, 1.0f },
	{ "END", nullptr }
};


//=============================================================================================
//		初	期	化
void SE::Initialize()
{
	play_manager = new iexSoundIIDX;

	data_no = 0;
	int set_num = 0;
	max_count = 0;

	while (true)
	{
		if (strcmp(all_se_data[data_no].id, "END") == 0) break;	// 終端
		
		// 基準位置保存
		all_se_data[data_no].start_num = set_num;

		play_manager->Set_copy(all_se_data[data_no].file_name, all_se_data[data_no].start_num, all_se_data[data_no].same_play_max);
		set_num += all_se_data[data_no].same_play_max;

		data_no++;
	}

	max_count = set_num;
}
//
//=============================================================================================



//=============================================================================================
//		解		放
void SE::Release()
{
	delete play_manager;
}
//
//=============================================================================================



//=============================================================================================
//		更		新
void SE::Update()
{

}
//
//=============================================================================================



//=============================================================================================
//		処		理
int SE::Play_in(int no, float volume, bool loop)
{
	int played = -1;	// データ見つからないか再生領域満タンなら-1が返る

	if (no != NOT_FOUND)
	{
		// 再生していない領域を探す
		for (int found = all_se_data[no].start_num; found < all_se_data[no].start_num + all_se_data[no].same_play_max; found++)
		{
			if (!play_manager->isPlay(found)){	// 見つかった！
				const int vol = (int)(-3000 * (1.0f - volume));
				play_manager->SetVolume(found, vol);
				play_manager->Play(found, loop);
				played = found;
				break;	// 見つかったのでループ抜けます(書くの忘れると悲惨)
			}
		}
	}

	return played;
	//return data_num;
}

int SE::Play(char *_ID, bool loop)
{
	const int data_num = Find_data_no(_ID);	// 区切るためのID

	return Play_in(data_num, all_se_data[data_num].volume, loop);
}

int SE::Play(char *_ID, float volume, bool loop)
{
	const int data_num = Find_data_no(_ID);	// 区切るためのID

	if (volume < 0) volume = 0;
	else if (volume > 1.0f) volume = 1.0f;

	return Play_in(data_num, volume, loop);
}

void SE::Stop(int no)
{
	play_manager->Stop(no);
}

void SE::Stop_all()
{
	for (int i = 0; i < max_count; i++) play_manager->Stop(i);
}
//
//=============================================================================================


//=============================================================================================
//		ＩＤ一致データ検索
int	SE::Find_data_no(char *_ID)
{
	int	result = NOT_FOUND;
	// データ数分ループ
	for (int i = 0; i < data_no; i++)
	{
		// 判定
		if (strcmp(all_se_data[i].id, _ID) != 0) continue;

		// 発見
		result = i;
		// ループを抜ける
		break;
	}

	return result;
}
//
//=============================================================================================




//********************************************************************************************************
//
//									B	G	M
//
//********************************************************************************************************

//*********************************************************************************************
//		パラメータの設定
//*********************************************************************************************
//	BGMデータ(textで読み込むのも良いかもしれない)
BGM::DATA	all_bgm_data[] =
{
	{ "バトル系", "DATA/Sound/BGM/フライ・ハイ.wav", 1.0f, true, BGM::FADE_NONE },
	{ "ほのぼの系", "DATA/Sound/BGM/ロデオ牧場.wav", 1.0f, true, BGM::FADE_NONE },
	{ "END", nullptr }
};


//=============================================================================================
//		初	期	化
void BGM::Initialize()
{
	play_manager = new iexSoundIIDX;

	data_no = 0;

	while (true)
	{
		if (strcmp(all_bgm_data[data_no].id, "END") == 0) break;	// 終端

		play_manager->Set(data_no, all_bgm_data[data_no].file_name);
		data_no++;
	}


	// 関数ポインタ
	Fade_mode_funk[FADE_NONE] = &BGM::None;
	Fade_mode_funk[FADE_IN] = &BGM::In;
	Fade_mode_funk[FADE_OUT] = &BGM::Out;
}
//
//=============================================================================================



//=============================================================================================
//		解		放
void BGM::Release()
{
	delete play_manager;
}
//
//=============================================================================================



//=============================================================================================
//		更		新
void BGM::Update()
{
	for (int i = 0; i < BGM::data_no; i++)
	{
		(this->*Fade_mode_funk[all_bgm_data[i].fade_type])(i);	// フェード関数実行
	}
}
//
//=============================================================================================



//=============================================================================================
//		処		理
void BGM::None(int no)
{}

void BGM::In(int no)
{
	// ボリューム上げていく
	all_bgm_data[no].volume += all_bgm_data[no].fade_speed;
	const int vol = (int)(-3000 * (1.0f - all_bgm_data[no].volume));

	// フェードしきった！
	if (all_bgm_data[no].volume > 1.0f)
	{
		all_bgm_data[no].volume = 1.0f;
		all_bgm_data[no].fade_type = FADE_NONE;
	}

	play_manager->SetVolume(no, vol);
}

void BGM::Out(int no)
{
	// ボリューム下げていく
	all_bgm_data[no].volume -= all_bgm_data[no].fade_speed;
	const int vol = (int)(-3000 * (1.0f - all_bgm_data[no].volume));

	// フェードしきった！
	if (all_bgm_data[no].volume < 0.0f)
	{
		all_bgm_data[no].volume = 0.0f;
		all_bgm_data[no].fade_type = FADE_NONE;
	}

	play_manager->SetVolume(no, vol);
}

int BGM::Play_in(int no, float volume, bool loop)
{
	if (no != NOT_FOUND)
	{
		const int vol = (int)(-3000 * (1.0f - volume));
		play_manager->SetVolume(no, vol);
		play_manager->Play(no, loop);
	}

	return no;
}

int BGM::Play(char *_ID, bool loop)
{
	const int no = Find_data_no(_ID);

	return Play_in(no, all_bgm_data[no].volume, loop);
}

int BGM::Play(char *_ID, float volume, bool loop)
{
	const int no = Find_data_no(_ID);

	if (volume < 0) volume = 0;
	else if (volume > 1.0f) volume = 1.0f;

	return Play_in(no, volume, loop);
}

void BGM::Stop(char *_ID)
{
	int no = Find_data_no(_ID);
	if (no == NOT_FOUND) return;
	play_manager->Stop(no);
}

int BGM::Fade_in(char *_ID, int speed, bool loop)
{
	/*	呼び出しは1回だけでお願いします	*/

	int no = Find_data_no(_ID);
	if (no == NOT_FOUND) return NOT_FOUND;
	all_bgm_data[no].volume = .0f;
	all_bgm_data[no].fade_type = F_TYPE::FADE_IN;
	if (speed <= 0) speed = 1;
	all_bgm_data[no].fade_speed = (speed * .01f);
	play_manager->Play(no, loop);
	play_manager->SetVolume(no, -10000);

	return no;
}

void BGM::Fade_out(char *_ID, int speed)
{
	/*	呼び出しは1回だけでお願いします	*/

	int no = Find_data_no(_ID);
	if (no == NOT_FOUND) return;
	all_bgm_data[no].volume = 1.0f;
	all_bgm_data[no].fade_type = F_TYPE::FADE_OUT;
	if (speed <= 0) speed = 1;
	all_bgm_data[no].fade_speed = (speed * .01f);
}

void BGM::Stop_all()
{
	for (int i = 0; i < data_no; i++) play_manager->Stop(i);
}
//
//=============================================================================================











//=============================================================================================
//		ＩＤ一致データ検索
int	BGM::Find_data_no(char *_ID)
{
	int	result = NOT_FOUND;
	// データ数分ループ
	for (int i = 0; i < data_no; i++)
	{
		// 判定
		if (strcmp(all_bgm_data[i].id, _ID) != 0) continue;

		// 発見
		result = i;
		// ループを抜ける
		break;
	}

	return result;
}
//
//=============================================================================================



//=============================================================================================
//		実		体
SE *se;
BGM *bgm;
//
//=============================================================================================