#include	"iextreme.h"
#include	"../system/system.h"
#include	"BasePlayer.h"
#include	"MyPlayer.h"
#include	"../ui/UI.h"
#include	"../../IEX/OKB.h"
#include	"../../../share_data/Enum_public.h"
#include	"../Sound/SoundManager.h"
#include "../stage/Stage.h"
#include "../Animation/Spread2D.h"
#include	"../Manhole/Manhole.h"
#include	"../Barrier/Barrier.h"
#include	"../paper object/paper object manager.h"
#include	"../paper object/Rend data.h"
#include	"../Rush/Rush.h"
#include	"../SkillBegin/SkillBegin.h"
#include	"../Effect/Effect.h"
#include	"../blur/blur.h"
#include	"../HitEffect/HitEffect.h"
#include	"../Ambulance/Ambulance.h"
#include	"../Scatter/Scatter.h"
static const bool on_number = true;	// ナンバーエフェクト自分で出すかどうか

//****************************************************************************************************************
//
//  初期化
//
//****************************************************************************************************************
MyPlayer::MyPlayer() :BasePlayer()
{
	//skillGage = new Pie_graph_content("DATA/skillGage/SpiritCircle_gage.png");	//	ゲージ
	//skillGage->Add_content("DATA/skillGage/SpiritCircle_UNDER.png");

	// 関数ポインタ初期化
	RendSE[KIND_PAPER_OBJECT::POSTER] = &MyPlayer::RendPosterSE;
	RendSE[KIND_PAPER_OBJECT::FLYER] = &MyPlayer::RendFlyerSE;
	RendSE[KIND_PAPER_OBJECT::CALENDAR] = &MyPlayer::RendCalendarSE;
	RendSE[KIND_PAPER_OBJECT::MAGAZIN] = &MyPlayer::RendMagazineSE;
	RendSE[KIND_PAPER_OBJECT::MONEY] = &MyPlayer::RendMoneySE;
	RendSE[KIND_PAPER_OBJECT::SEISHO] = &MyPlayer::RendSeisyoSE;
	RendSE[KIND_PAPER_OBJECT::SHINBUN] = &MyPlayer::RendShinbunSE;
	RendSE[KIND_PAPER_OBJECT::SIGN] = &MyPlayer::RendSignSE;
	RendSE[KIND_PAPER_OBJECT::TOILET_PAPER] = &MyPlayer::RendToileSE;
	RendSE[KIND_PAPER_OBJECT::ZASSHI] = &MyPlayer::RendZasshiSE;
	RendSE[KIND_PAPER_OBJECT::SHOJI] = &MyPlayer::RendShojiSE;
	RendSE[KIND_PAPER_OBJECT::HUUSENN] = &MyPlayer::RendBalloonSE;
	RendSE[KIND_PAPER_OBJECT::KAOPANEL] = &MyPlayer::RendPanelSE;
}

MyPlayer::~MyPlayer()
{
	Release();
}

void MyPlayer::Initialize(iex3DObj **obj)
{
	BasePlayer::Initialize(obj);
	isMyNunber = true;
	se_step = 0;

	ShowCursor(FALSE);

	hit_effect = new HitEffect;
	scatter = new Scatter;

	// 破るマウスの動きの初期化
	const int num = 4;
	Rend_data::Movedata data[num]=
	{
		{ 80000, 0 },
		{ -80000, 0 },
		{ 80000, 0 },
		{ -80000, 0 }
	};

	// commandデータ
	//command_data = new Rend_data*[KIND_PAPER_OBJECT::KIND_MAX_PAPER_OBJ];
	//Rend_data::Load(command_data[KIND_PAPER_OBJECT::CALENDAR],	"DATA/paper object/calendar/rend command.txt");
	//Rend_data::Load(command_data[KIND_PAPER_OBJECT::FLYER],		"DATA/paper object/flyer/rend command.txt");
	//Rend_data::Load(command_data[KIND_PAPER_OBJECT::MAGAZIN],	"DATA/paper object/magazin/rend command.txt");
	//Rend_data::Load(command_data[KIND_PAPER_OBJECT::MONEY],		"DATA/paper object/money/rend command.txt");
	//Rend_data::Load(command_data[KIND_PAPER_OBJECT::POSTER],	"DATA/paper object/Poster/rend command.txt");
	//Rend_data::Load(command_data[KIND_PAPER_OBJECT::SEISHO],	"DATA/paper object/seisho/rend command.txt");
	//Rend_data::Load(command_data[KIND_PAPER_OBJECT::SHINBUN],	"DATA/paper object/shinbun/rend command.txt");
	//Rend_data::Load(command_data[KIND_PAPER_OBJECT::SHOJI],		"DATA/paper object/shoji/rend command.txt");
	//Rend_data::Load(command_data[KIND_PAPER_OBJECT::SIGN],		"DATA/paper object/sign/rend command.txt");
	//Rend_data::Load(command_data[KIND_PAPER_OBJECT::TOILET_PAPER], "DATA/paper object/toilet_paper/rend command.txt");
	//Rend_data::Load(command_data[KIND_PAPER_OBJECT::ZASSHI],	"DATA/paper object/zasshi/rend command.txt");
}

void MyPlayer::Release()
{
	delete hit_effect;
	delete scatter;
	//BasePlayer::Release();
}




//*************************************************************************************************************************
//		更新
//*************************************************************************************************************************

void MyPlayer::Update()
{
	if (ui->isStart())
	{
		/*入力受付処理*/
		Control_all();
		/*マウスの処理*/
		Mouse_Update();

		// ATフィールドセット
		Vector3 AT_nearest;
		stage->Area_Get_nearest_point(0, &AT_nearest, pos);
		se->Set_pos("AT", 0, AT_nearest);

		//救急車衝突
		if (ambulance_mng->CheckDist(pos, 20.0f) != -1)
		{
			if (!(m_controlDesc.controlFlag & BYTE(PLAYER_CONTROL::AMBULANCE)))se->Play("救急車衝突");
			m_controlDesc.controlFlag |= (BYTE)PLAYER_CONTROL::AMBULANCE;
		}
	}

	// ヒットエフェクト更新
	const Vector3 shift(sinf(angleY) * 5, 5, cosf(angleY) * 5);
	hit_effect_pos = pos + shift;
	hit_effect->Update(hit_effect_pos);
	scatter->Update(pos+Vector3(0,12,0), Vector3(0, angleY+PI, 0), 0.075f);

	particle_pos = Vector3(pos.x + sinf(angleY) * 3, pos.y, pos.z + cosf(angleY) * 3);

	if (KEY(KEY_ENTER) == 3)
	{
		//Rend_effect(hit_effect_pos, 3.0f);
	}
	
	BasePlayer::Update();
	Update_action();

	// リスナー情報
	Update_listener();

	//if (rend_data->Move_check(m_controlDesc.mouseX, m_controlDesc.mouseY))
	//{
	//	se->Play("手裏剣");
	//	rend_data->Reset();
	//}
}


//*************************************************************************************************************************
//		描画
//*************************************************************************************************************************
void MyPlayer::Render(iexShader *shader, char *name)
{
	BasePlayer::Render(shader, name);
	//Text::Draw(320, 480, 0xff00ffff, "%.3f", m_controlDesc.mouseX);
	//Text::Draw(320, 520, 0xff00ffff, "%.3f", m_controlDesc.mouseY);
	//Text::Draw(32, 560, 0xff00ffff, "%.1f",pos.y);
	//Text::Draw(32, 600, 0xff00ffff, "%.1f",pos.z);
	//DeferredManager.ForwardBigin();

	//円ゲージ
	//if (manhole_mng->CheckManhole(ManholeMng::LAND_TYPE::TIJOU, pos, 30) != -1)Text::Draw(32, 420, 0xff00ffff, "やったぜ");
	//skillGage->Render(persent, 0, 300, 128, 128, 0, 0, 128, 128);
	//DeferredManager.ForwardEnd();
}

void MyPlayer::Render_forword()
{
	hit_effect->Render();
	BasePlayer::Render_forword();
	scatter->Render();
}

void MyPlayer::Update_action()
{
	// 継承先のアクションクラスを使えないのでここで無理やり更新
	ui->Set_rend_command(UI::REND_COMMAND::NONE);

	switch (action_part)
	{
	case ACTION_PART::REND:
		//// マウス離したらリセット
		//if (!(m_controlDesc.controlFlag&(BYTE)PLAYER_CONTROL::LEFT_CLICK))
		//{
		//	command_data[paper_obj_mng->Get_kind(poster_num)]->Reset();
		//	return;
		//}
		//
		///* コマンドマークをUIクラスに送る */
		//float move_x, move_y;
		//command_data[paper_obj_mng->Get_kind(poster_num)]->Get_commnand(&move_x, &move_y);
		//Vector2 com_move((int)move_x, (int)move_y);
		//com_move.Normalize(move_x, move_y);
		//UI::REND_COMMAND com;
		//if (move_x == 0)
		//{
		//	com = (move_y < 0) ? UI::REND_COMMAND::DOWN : UI::REND_COMMAND::UP;
		//}
		//else if (move_y == 0)
		//{
		//	com = (move_x < 0) ? UI::REND_COMMAND::LEFT : UI::REND_COMMAND::RIGHT;
		//}
		//else
		//{
		//	int i = 0;
		//}
		//ui->Set_rend_command(com);
		//
		//// マウススライドコマンド入力しきったら
		//if (command_data[paper_obj_mng->Get_kind(poster_num)]->Move_check(m_controlDesc.mouseX, m_controlDesc.mouseY))
		//{
		//	m_controlDesc.rendFlag |= (BYTE)PLAYER_FLAG::REND;
		//	command_data[paper_obj_mng->Get_kind(poster_num)]->Reset();
		//}

		if (sqrtf(m_controlDesc.mouseX*m_controlDesc.mouseX + (m_controlDesc.mouseY*1.1f)*(m_controlDesc.mouseY*1.1f)) > 80000) m_controlDesc.rendFlag |= (BYTE)PLAYER_FLAG::REND;

		break;


	case ACTION_PART::MANHOLE:
		if (models[(int)model_part]->GetParam(0) == 1)
		{
			if (se_receive == -1)
			{
				se_receive = se->Play((isManhole) ? "のぼる" : "落ちる");
			}
		}
		if (models[(int)model_part]->GetParam(0) == 2)
		{
			if (ui->GetManholeFade() != UI::MANHOLE_FADE_TYPE::F_OUT)
				ui->SetManholeFade(UI::MANHOLE_FADE_TYPE::F_OUT);
		}
		break;

	case ACTION_PART::SYURIKEN:
	{
								  static bool stop = false;
								  if (move.LengthSq() < 1)
								  {

									  if (!stop) BlurFilter::Set(8, 0, 0);
									  stop = true;
								  }
								  else stop = false;
	}

		break;

	case ACTION_PART::REND_OBJ:
		if (kind_paper_obj != -1)(this->*RendSE[kind_paper_obj])();	// それぞれのモーションに合わせて音を出す
		break;
	}
}


/*	操作　*/
void MyPlayer::Control_all()
{
	// 初期化
	m_controlDesc.moveFlag &= 0x00000000;
	m_controlDesc.controlFlag &= 0x00000000;

	static int skill_wait = 0;
	if (m_controlDesc.skillFlag != 0)
	{
		if (--skill_wait <= 0){ skill_wait = 0; m_controlDesc.skillFlag &= 0x00000000; }
	}

	if (KEY_Get(KEY_UP) == 1)
	{
		m_controlDesc.moveFlag |= (BYTE)PLAYER_IMPUT::UP;
	}
	else if (KEY_Get(KEY_DOWN) == 1)
	{
		m_controlDesc.moveFlag |= (BYTE)PLAYER_IMPUT::DOWN;
	}
	if (KEY_Get(KEY_RIGHT) == 1)
	{
		m_controlDesc.moveFlag |= (BYTE)PLAYER_IMPUT::RIGHT;
	}
	else if (KEY_Get(KEY_LEFT) == 1)
	{
		m_controlDesc.moveFlag |= (BYTE)PLAYER_IMPUT::LEFT;
	}

	//if (GetAsyncKeyState(0x01) & 0x8000)
	if (KeyBoard(MOUSE_LEFT))
	{
		m_controlDesc.controlFlag |= (BYTE)PLAYER_CONTROL::LEFT_CLICK;
	}

	else if (KeyBoard(MOUSE_RIGHT))
	{
		m_controlDesc.controlFlag |= (BYTE)PLAYER_CONTROL::RIGHT_CLICK;
	}
	else if (KeyBoard(MOUSE_CENTAR))
	{
		m_controlDesc.controlFlag |= (BYTE)PLAYER_CONTROL::ATTACK_BUTTON;
	}

	if (KeyBoard(KB_SPACE))
	{
		m_controlDesc.controlFlag |= (BYTE)PLAYER_CONTROL::SPACE;
	}

	// 右クリックで必殺技を発動させるか
	if (m_controlDesc.controlFlag & (int)PLAYER_CONTROL::RIGHT_CLICK)
	{
		if (action_part != ACTION_PART::MOVE) return;
		// ゲージが溜まってたら
		if (skill_data[select_skill].wait_time <= 0)
		{
			const int FLAG[] =
			{
				(int)PLAYER_SKILL::GUN,
				(int)PLAYER_SKILL::SYURIKEN,
				(int)PLAYER_SKILL::KABUTO,
			};

			m_controlDesc.skillFlag |= FLAG[select_skill];
			// スキル撃ったのでクールタイム設定
			skill_data[select_skill].wait_time = skill_data[(int)select_skill].cool_time;
			skill_wait = 3;	// 1フレームだけしか送らなかったらたまに反応しないので3フレームぐらい送る

			for (int i = 0; i < (int)SKILL::MAX; i++)
			{
				if (!skill_data[i].unlock) continue;
				if (skill_data[i].cool_time <= 0)
				{
					select_skill = i;
					break;
				}
			}
		}
		//SPI_GET_WHEELSCROLL
	}
}

float MOUSE_SENS = 0.8f;	// マウスの感度

/*	マウス更新	*/
void MyPlayer::Mouse_Update()
{
	//	キー設定	カーソル
	GetCursorPos(&mousePos);

	//　画面の真ん中へ移動
	RECT rc;
	GetWindowRect(iexSystem::Window, &rc);

	mousePos.x -= rc.left + iexSystem::ScreenWidth / 2;
	mousePos.y -= rc.top + iexSystem::ScreenHeight / 2;
	//if (mousePos.x < -128) mousePos.x = -128;
	//if (mousePos.x >  127) mousePos.x = 127;
	//if (mousePos.y < -128) mousePos.y = -128;
	//if (mousePos.y >  127) mousePos.y = 127;
	mousePos.x = (LONG)((float)mousePos.x * MOUSE_SENS);
	mousePos.y = (LONG)((float)mousePos.y * MOUSE_SENS);
	if (GetActiveWindow() == IEX_GetWindow())
	{
		ShowCursor(FALSE);
		SetCursorPos(rc.left + iexSystem::ScreenWidth / 2, rc.top + iexSystem::ScreenHeight / 2);
	}

	//　マウスの動きをデスクに渡す
	m_controlDesc.mouseX = (float)mousePos.x * 1000;
	m_controlDesc.mouseY = (float)mousePos.y * 1000;

	// ホイール
	if (Get_wheel_flag() == WHEEL_FLAG::DOWN)
	{
		if (++select_skill < (int)SKILL::MAX)
		{
			if (skill_data[select_skill].unlock) return;
		}
		select_skill = 0;
	}
	else if (Get_wheel_flag() == WHEEL_FLAG::UP)
	{
		if (--select_skill < 0)
		{
			for (int i = (int)SKILL::MAX - 1;; i--)
			{
				if (skill_data[i].unlock) { select_skill = i; break; }
			}
		}
	}
}

void MyPlayer::Update_listener()
{
	const Vector3 front(models[(int)model_part]->TransMatrix._31, models[(int)model_part]->TransMatrix._32, models[(int)model_part]->TransMatrix._33);
	const Vector3 up(models[(int)model_part]->TransMatrix._21, models[(int)model_part]->TransMatrix._22, models[(int)model_part]->TransMatrix._23);

	se->Set_listener(pos, front, up, move);
}

void MyPlayer::RendPosterSE()
{
	switch (se_step)
	{
	case 0:
		// 破き始め
		if (models[(int)model_part]->GetParam(0) == 1)
		{
			if (poster_num != -1)
			{
				Rend_effect(hit_effect_pos, 2.0f, 5);
				Rend_effect2(hit_effect_pos, 2.0f, 10);
				hit_effect->Action(HitEffect::HIT_TYPE::ALL);
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 20, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			se_receive = se->Play("破る");
			se_step = 99;	// ステップ終わり
		}
		break;
	}
}
void MyPlayer::RendFlyerSE()
{
	switch (se_step)
	{
	case 0:
		// 破き始め
		if (models[(int)model_part]->GetParam(0) == 1)
		{
			if (poster_num != -1)
			{
				Rend_effect(hit_effect_pos, 2.0f, 5);
				Rend_effect2(hit_effect_pos, 2.0f, 10);
				hit_effect->Action(HitEffect::HIT_TYPE::ALL);
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 20, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			se_receive = se->Play("破る");
			se_step = 99;	// ステップ終わり
		}
		break;
	}
}
void MyPlayer::RendCalendarSE()
{
	switch (se_step)
	{
	case 0:
		// 破き始め
		if (models[(int)model_part]->GetParam(5) == 1)
		{
			BlurFilter::Set(12, 0, 0);
			scatter->Action();
			se_receive = se->Play("カレンダー破り", true);
			se_step++;
		}
		break;
	case 1:
		// 紙いっぱい飛んでる
		Rend_effect(particle_pos, 3.0f, 1);

		// 破き終わり
		if (models[(int)model_part]->GetParam(5) == 2)
		{
			Rend_effect2(particle_pos, 3.0f);
			se->Stop("カレンダー破り", se_receive);
			se_step++;
		}
		break;
	case 2:
		// 雄叫び
		if (models[(int)model_part]->GetParam(5) == 3)
		{
			if (poster_num != -1)
			{
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 10, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			se_step = 99;	// ステップ終わり
		}
	}
}
void MyPlayer::RendMagazineSE()
{
	switch (se_step)
	{
	case 0:
		// 最初から鳴らす
		se_receive = se->Play("マガジン破り");
		se_step++;

		break;
	case 1:
		// 破き
		if (models[(int)model_part]->GetParam(5) == 1)
		{
			if (poster_num != -1)
			{
				Rend_effect(hit_effect_pos, 3.0f);
				Rend_effect2(hit_effect_pos, 3.0f, 10);
				hit_effect->Action(HitEffect::HIT_TYPE::ALL);
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 10, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			se->Stop("マガジン破り", se_receive);
			se->Play("マガジン破り2");
			se_step = 99;	// ステップ終わり
		}
		break;
	}
}
void MyPlayer::RendMoneySE()
{
	switch (se_step)
	{
	case 0:
		// 破き始め
		if (models[(int)model_part]->GetParam(5) == 1)
		{
			BlurFilter::Set(12, 0, 0);
			scatter->Action();
			se_receive = se->Play("お金破り", true);
			se_step++;
		}
		break;
	case 1:
		// 紙いっぱい飛んでる
		Rend_effect(hit_effect_pos+Vector3(0,2,0), 2.0f, 1);
		Rend_effect2(hit_effect_pos + Vector3(0, 2, 0), 2.0f, 1);

		// 破き終わり
		if (models[(int)model_part]->GetParam(5) == 2)
		{
			se->Stop("お金破り", se_receive);
			se_step++;
		}
		break;
	case 2:
		// 投げ捨て
		if (models[(int)model_part]->GetParam(5) == 3)
		{
			if (poster_num != -1)
			{
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 10, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			se->Play("サイン破り2");
			se_step = 99;	// ステップ終わり
		}
	}
}
void MyPlayer::RendSeisyoSE()
{
	switch (se_step)
	{
	case 0:
		// 最初から鳴らす
		se_receive = se->Play("聖書破り");
		se_step++;
		break;

	case 1:
		// 上に投げ
		if (models[(int)model_part]->GetParam(5) == 1)
		{
			se->Play("聖書破り2");
			se_step++;	// ステップ終わり
		}
		break;

	case 2:
		// 破き
		if (models[(int)model_part]->GetParam(5) == 2)
		{
			if (poster_num != -1)
			{
				hit_effect->Action(HitEffect::HIT_TYPE::SLASH);
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 10, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			se->Stop("聖書破り", se_receive);
			se->Play("聖書破り3");
			se_step = 99;	// ステップ終わり
		}
		break;
	}
}
void MyPlayer::RendSignSE()
{
	switch (se_step)
	{
	case 0:
		// 破き始め
		if (models[(int)model_part]->GetParam(5) == 1)
		{
			if (poster_num != -1)
			{
				hit_effect->Action();
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 10, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			se->Play("サイン破り");
			se_step++;
		}
		break;
	case 1:
		// 破き終わり
		if (models[(int)model_part]->GetParam(5) == 2)
		{
			//se->Play("サイン破り2");
			se->Play("短い破り");
			se_step = 99;	// ステップ終わり
		}
		break;
	}
}
void MyPlayer::RendShinbunSE()
{
	switch (se_step)
	{
	case 0:
		// 紙の音的な
		se_step++;
		break;

	case 1:
		// 破き始め
		if (models[(int)model_part]->GetParam(5) == 1)
		{
			if (poster_num != -1)
			{
				Rend_effect(hit_effect_pos, 3.0f, 20);
				Rend_effect2(hit_effect_pos, 3.0f, 5);
				hit_effect->Action(HitEffect::HIT_TYPE::ALL);
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 10, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			se->Play("新聞破り");
			se_step++;
		}
		break;
	case 2:
		// 破き終わり
		if (models[(int)model_part]->GetParam(5) == 2)
		{
			se->Play("新聞破り2");
			se_step = 99;	// ステップ終わり
		}
		break;
	}
}
void MyPlayer::RendToileSE()
{
	switch (se_step)
	{
	case 0:
		// 最初から鳴らす
		se_receive = se->Play("トイレ破り");
		se_step++;

		break;
	case 1:
		// 破き
		if (models[(int)model_part]->GetParam(5) == 1)
		{
			if (poster_num != -1)
			{
				hit_effect->Action(HitEffect::HIT_TYPE::ALL);
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 10, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			Rend_effect2(hit_effect_pos, 3.0f, 30);
			se->Stop("トイレ破り", se_receive);
			se->Play("トイレ破り2");
			se_step = 99;	// ステップ終わり
		}
		break;
	}
}
void MyPlayer::RendZasshiSE()
{
	switch (se_step)
	{
	case 0:
		// 破き始め
		if (models[(int)model_part]->GetParam(5) == 1)
		{
			if (poster_num != -1)
			{
				hit_effect->Action(HitEffect::HIT_TYPE::ALL);
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 10, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			Rend_effect(hit_effect_pos, 3.0f, 20);
			Rend_effect2(hit_effect_pos, 3.0f, 10);
			se_receive = se->Play("短い破り");
			se_step = 99;	// ステップ終わり
		}
		break;
	}
}

void MyPlayer::RendShojiSE()
{
	switch (se_step)
	{
	case 0:
		// 破き始め
		if (models[(int)model_part]->GetParam(5) == 1)
		{
			se_receive = se->Play("カレンダー破り", true);
			se_step++;
		}
		break;
	case 1:
		// 紙いっぱい飛んでる
		Rend_effect(hit_effect_pos, 2.0f, 2);
		Rend_effect2(hit_effect_pos, 2.0f, 1);

		// 破き終わり
		if (models[(int)model_part]->GetParam(5) == 2)
		{
			se->Stop("カレンダー破り", se_receive);
			se_step++;
		}
		break;
	case 2:
		// 蹴り
		if (models[(int)model_part]->GetParam(5) == 3)
		{
			if (poster_num != -1)
			{
				hit_effect->Action();
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 10, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			se->Play("サイン破り");
			se_step = 99;	// ステップ終わり
		}
	}
}

void MyPlayer::RendBalloonSE()
{
	switch (se_step)
	{
	case 0:
		// 破き始め
		if (models[(int)model_part]->GetParam(0) == 1)
		{
			if (poster_num != -1)
			{
				hit_effect->Action(HitEffect::HIT_TYPE::ALL);
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 20, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			se_receive = se->Play("手裏剣破り");
			se_step = 99;	// ステップ終わり
		}
		break;
	}
}

void MyPlayer::RendPanelSE()
{
	switch (se_step)
	{
	case 0:
		se_receive = se->Play("トイレ破り");
		se_step++;
		break;

	case 1:
		// 破き始め
		if (models[(int)model_part]->GetParam(5) == 1)
		{
			Rend_effect(hit_effect_pos, 3.0f, 10);
			hit_effect->Action(HitEffect::HIT_TYPE::ALL);
			se->Play("新聞破り");
			se->Play("聖書破り3");
			se->Stop("トイレ破り", se_receive);
			BlurFilter::Set(8, 0, 0);
			se_step++;
		}
		break;
	case 2:
		// 破き終わり
		if (models[(int)model_part]->GetParam(5) == 2)
		{
			se->Play("ジャンプ", pos);
			se->Play("落ちる");
			se_step++;
		}
		break;
	case 3:
		// 蹴り
		if (models[(int)model_part]->GetParam(5) == 3)
		{
			if (poster_num != -1)
			{
				BlurFilter::Set(4, 0, 0);
				hit_effect->Action((HitEffect::HIT_TYPE::ALL ^ HitEffect::HIT_TYPE::SLASH));
				if (on_number)Number_Effect::SetNum(paper_obj_mng->Get_pos(poster_num) + Vector3(0, 10, 0), paper_obj_mng->Get_point(poster_num), 4);
			}
			se->Play("サイン破り");
			se_step = 99;	// ステップ終わり
		}
	}
}

void MyPlayer::Set_action(ACTION_PART part)
{
	if (action_part == ACTION_PART::START)
		return;

	if (action_part != part)
	{
		if (part == ACTION_PART::TRANS_FORM)
		{
			skill_begin->Action();
			se->Play("スキル発動");
		}
		if (action_part == ACTION_PART::SYURIKEN)
		{
			// 手裏剣エフェクトストップ
			rush->Stop();
			Sand_effect(pos.x, pos.y, pos.z);
		}
		if (part == ACTION_PART::DIE)
		{
			if (isManhole)
			{
				isManhole = false;
				event_bgm->Manhole_off();
			}
		}

		if (action_part == ACTION_PART::MANHOLE && part == ACTION_PART::MOVE)
		{
			isManhole ^= 1;
			(!isManhole) ? event_bgm->Manhole_off() : event_bgm->Manhole_on(manhole_mng->CheckManhole(ManholeMng::LAND_TYPE::TIKA, pos, 30));
			ui->SetManholeFade(UI::MANHOLE_FADE_TYPE::F_IN);
		}
		if (part == ACTION_PART::REND_OBJ) se_step = 0;
		Change_action(part);
	}
}
