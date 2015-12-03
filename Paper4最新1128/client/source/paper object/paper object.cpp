
#include "iextreme.h"
#include "paper object.h"

Paper_obj::Paper_obj() : model(nullptr), position(0, 0, 0), angle(0)
{}

Paper_obj::~Paper_obj()
{
	delete model;
}

void Paper_obj::Initialize(iex3DObj *model)
{
	this->model = model->Clone();
}

void Paper_obj::Set_pose(const float angle, const Vector3& pos)
{
	this->angle = angle;
	model->SetAngle(angle);

	this->position = pos;
	model->SetPos(position);
}

void Paper_obj::Set_user(int number)
{
	this->number = number;
}

void Paper_obj::Set_animframe(int frame)
{
	this->model->SetFrame(frame);
}

const Vector3 &Paper_obj::Get_pos()
{
	return position;
}

void Paper_obj::Render_mark(iex2DObj *obj, bool target)
{
	float tu[2];
	(target) ? tu[0] = 0.5f, tu[1] = 1 : tu[0] = 0, tu[1] = 0.5f;
	float tv[2] = { 0, 1 };
	Billboard::Draw3D(position + Vector3(0, 5, 0), obj, 256, 256, tu, tv, RS_COPY);
}