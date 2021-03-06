
#include "Stage.h"
#include "Area.h"
#include "../fstream/fstream_paper.h"
#include <string>
#include "Inhabitant.h"
#include "../Player/PlayerManager.h"

Stage::Stage() : area( nullptr )
{}

Stage::~Stage()
{
	Release();
}

void Stage::Initialize()
{
	//show_model = new iexMesh("DATA/MATI/stage_machi.IMO");
	//collision_model = new iexMesh("DATA/MATI/stage_atari.IMO");

	//show_model = new iexMesh("../stage_machi.IMO");

	collision_model.push_back( new iexMesh( "DATA/MATI/stage_atari.IMO" ) );

	
	Load_mapdata();


	for( unsigned int i = 0; i < show_model.size(); i++ )
	{
		show_model[i]->model->Update();
	}

	inhabitants = new Inhabitants( "DATA/MATI/position/zattou.txt" );

	Load_area();
}

void Stage::Release()
{
	for( unsigned int i = 0; i < show_model.size(); i++ )
		delete show_model[i];
	show_model.clear();
	for( unsigned int i = 0; i < collision_model.size(); i++ )
		delete collision_model[i];
	collision_model.clear();
	delete area; area = nullptr;
	delete inhabitants; inhabitants = nullptr;
}

void Stage::Update()
{
	area->Update();
	inhabitants->Update();
}

void Stage::Render( iexShader *shader, char *name )
{
	for( unsigned int i = 0; i < show_model.size(); i++ )
	{
		show_model[i]->model->Render( shader, name );
	}
}

void Stage::Render_culling( const Vector3 &pos, const Vector3 &front, iexShader *shader, char *name )
{
	for( unsigned int i = 0; i < show_model.size(); i++ )
	{
		// viewfrontの方向に移動させる
		Vector3 modelpos = show_model[i]->model->GetPos() + front * show_model[i]->size;
		// 後ろは描画しない
		if( Vector3Dot( modelpos - pos, front ) < 0 )
			continue;

		show_model[i]->model->Render( shader, name );
	}
}

void Stage::Render_collision_model( iexShader *shader, char *name )
{
	for( unsigned int i = 0; i < collision_model.size(); i++ )
	{
		collision_model[i]->Render( shader, name );
	}
}

// エリアはGの対象外に
void Stage::RenderForward()
{

	area->Render();

}
// 町の人描画
void Stage::Render_inhabitants( iexShader *shader, char *name )
{
	inhabitants->Render( shader, name );
}


// 当たり判定

bool Stage::Collision_rand(const Vector3 &pos, Vector3 *move, float up)
{
	if (move->y >= 0)
		return false;

	Vector3 hit_pos, ray_pos(pos), vec(0, -1, 0);
	float dist(FLT_MAX);
	// xzだけ移動
	ray_pos += *move;
	ray_pos.y = pos.y + 3.0f;

	if (collision_model[0]->RayPickUD(&hit_pos, &ray_pos, &vec, &dist) == -1)
		return false;

	if (pos.y + move->y >= hit_pos.y)
		return false;

	move->y = hit_pos.y - pos.y;
	return true;
}

void Stage::Collision(const Vector3 &pos, Vector3 *move, float radius, int recursive_counter)
{
	if (recursive_counter <= 0)
		return;

	Vector3 move_xz(move->x, 0, move->z); // moveのxとzだけ

	Vector3 hit_pos, ray_pos, vec;
	float dist(FLT_MAX);
	// レイの始点を上げる
	ray_pos = pos;
	ray_pos.y += 1.0f;
	// レイの方向を決める
	vec = move_xz;
	vec.y = 0;
	vec.Normalize();

	if (collision_model[0]->RayPick(&hit_pos, &ray_pos, &vec, &dist) == -1)
		return;

	Vector3 next_pos(pos + move_xz);
	// 壁にめり込んだ量
	Vector3 sink(next_pos - hit_pos);
	// sinkの法線方向の大きさ
	float sink_nl;
	vec.y = 0;
	vec.Normalize();
	sink_nl = -Vector3Dot(sink, vec);

	if (sink_nl + radius <= 0)
		return;

	sink_nl += radius;
	*move += vec * sink_nl;
}



unsigned int Stage::Area_Get_numof()
{
	return area->Get_num();
}

void Stage::Area_Is_work(unsigned int index, bool in)
{
	area->Set_Is_work(index, in);
}

void Stage::Area_Get_nearest_point(unsigned int index, Vector3 *out, const Vector3 &pos)
{
	area->Get_nearest_point(index, out, pos);
}

void Stage::Load_mapdata()
{
	// テキストファイルを開く
	std::ifstream ifs( "DATA/MATI/position/stage.txt" );
	// 最後までループ
	while( ifs.eof() == false )
	{
		// メッシュファイルのパスを読む
		std::string filename;
		ifs >> filename;
		// IMOじゃなかったらとばす
		if( filename.find( "IMO" ) == std::string::npos )
			continue;
		// stringからchar配列に変換
		char c[64] = {};
		strcat_s( c, 64, filename.c_str() );
		// imoからメッシュ作成
		iexMesh* mesh = new iexMesh( c );
		// 失敗したらエラー
		if( mesh->GetMesh() == NULL )
		{
			char text[256] = {};
			sprintf_s( text, "%s 読み込み失敗", c );
			MessageBox( NULL, text, "エラー", 0 );
			continue;
		}
		// 配置する個数
		int num;
		ifs >> num;
		if( num == 0 )
			delete mesh;
		// num個配置
		for( int i = 0; i < num; i++ )
		{
			// 角度と位置を読む
			Vector3 angle, pos;
			ifs >> angle;
			angle *= 0.0174532924f; // 度からラジアンに変換
			ifs >> pos;
			// 座標系変換
			pos.x *= -1;
			angle *= -1;
			// メッシュ作成
			Show_model_part *smp;
			if( i == 0 ) // 最初の一つはそのまま
				smp = new Show_model_part( mesh );
			else         // 残りはクローン
			{
				// 応急処置(テクスチャが2以上貼り付けられてるモデルの場合、クローンするとエラーが出ます)
				if (strcmp(c, "DATA/MATI/manho-ru.IMO") == 0)
					smp = new Show_model_part(mesh->Clone(2));
				else
					smp = new Show_model_part(mesh->Clone());
			}
			// 角度と位置設定
			smp->model->SetAngle( angle.x, angle.y, angle.z );
			smp->model->SetPos( pos.x, pos.y, pos.z );
			// 更新
			smp->model->Update();
			// 配列に追加
			show_model.push_back( smp );
		}
	}
}

void Stage::Load_area()
{
	if( area )delete area;
	area = new Area_mng;

	std::ifstream ifs( "DATA/MATI/area/data.txt" );

	while( !ifs.eof() )
	{
		std::string str;
		ifs >> str;
		if( str.find( "IMO" ) != std::string::npos )
		{
			int time;
			ifs >> time;
			area->Push( str.c_str(), time );
		}
	}
}




Stage::Show_model_part::Show_model_part( iexMesh *model ) :model( model )
{
	size = model->Length_of_furthest_point();
}

Stage::Show_model_part::~Show_model_part()
{
	delete model;
}




Stage *stage = nullptr;
