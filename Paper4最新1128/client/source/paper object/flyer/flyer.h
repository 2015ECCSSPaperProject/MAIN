
//#include "iextreme.h"
//#include "../paper object.h"

#pragma once

enum KIND_PAPER_OBJECT;

class Flyer : public Paper_obj_3DObj
{
public:
	void Initialize( iex3DObj *model, int point, int start_time );
	void Subclass_update() override;
	void Subclass_render( iexShader *shader = nullptr, char *name = '\0' ) override;

	KIND_PAPER_OBJECT Get_kind();

	//**************************************************

	unsigned int Get_receive_data_size()override;

	void Set_receive_data( char *in )override;



private:
	float high; // 落下中の高さ
	int start_time; // 落下開始 秒
};
