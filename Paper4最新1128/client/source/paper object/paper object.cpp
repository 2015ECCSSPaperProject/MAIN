
#include "iextreme.h"
#include "paper object.h"
#include "../HitEffect/HitEffect.h"
#include "../Player/BasePlayer.h"
#include "../sound/SoundManager.h"
#include "../Effect/Effect.h"
Paper_obj::Paper_obj() : position(0, 0, 0), angle(0), broken(false), rend_delay_time(0)
{
	hiteffect = new HitEffect;
}

Paper_obj::~Paper_obj()
{
	delete hiteffect;
}

void Paper_obj::Update()
{
	Subclass_update();
	hiteffect->Update(this->position + Vector3(0.0f, 10.0f, 0.0f));
}

void Paper_obj::Render( iexShader *shader, char *name)
{
	Subclass_render( shader, name );
}

void Paper_obj::Forward_render()
{
	hiteffect->Render();
}

void Paper_obj::Set_pose(const float angle, const Vector3& pos)
{
	this->angle = angle;
	this->position = pos;
}

const Vector3 &Paper_obj::Get_pos()
{
	return position;
}

void Paper_obj::Rend()
{
	broken = true;
}

void Paper_obj::Rend_by_skill(BasePlayer *player)
{
	if (player->Get_action() == BasePlayer::ACTION_PART::GUN)
	{
		Rend_effect(position + Vector3(0, 10, 0), 3.0f, 10);
		Rend_effect2(position + Vector3(0, 10, 0), 3.0f, 20);
		hiteffect->Action(HitEffect::HIT_TYPE::ALL);
	}
	else if (player->Get_action() == BasePlayer::ACTION_PART::SYURIKEN)
	{
		Rend_effect(position + Vector3(0, 10, 0), 3.0f, 10);
		Rend_effect2(position + Vector3(0, 10, 0), 3.0f, 1);
		hiteffect->Action(HitEffect::HIT_TYPE::SLASH);
		se->Play("�藠���j��");
	}
	Number_Effect::SetNum(position + Vector3(0, 10, 0), point, 4);
	broken = true;
}

int Paper_obj::Get_point()
{
	return point;
};

bool Paper_obj::Can_rend()
{
	return (!broken);
}

Paper_obj_Mesh::Paper_obj_Mesh() : model( nullptr )
{}

Paper_obj_Mesh::~Paper_obj_Mesh()
{
	delete model;
}

void Paper_obj_Mesh::Initialize( iexMesh *model, int point )
{
	this->model = model->Clone();
	this->point = point;
}

void Paper_obj_Mesh::Set_pose( const float angle, const Vector3& pos )
{
	Paper_obj::Set_pose( angle, pos );
	model->SetAngle( angle );
	model->SetPos( position );
}



Paper_obj_3DObj::Paper_obj_3DObj() : model( nullptr )
{}

Paper_obj_3DObj::~Paper_obj_3DObj()
{
	delete model;
}

void Paper_obj_3DObj::Initialize( iex3DObj *model, int point )
{
	this->model = model->Clone();
	this->point = point;
}

void Paper_obj_3DObj::Set_pose( const float angle, const Vector3& pos )
{
	Paper_obj::Set_pose( angle, pos );
	model->SetAngle( angle );
	model->SetPos( position );
}

void Paper_obj_3DObj::Set_animframe( int frame )
{
	this->model->SetFrame( frame );
}
