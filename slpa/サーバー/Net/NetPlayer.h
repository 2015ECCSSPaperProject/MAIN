#pragma once
//********************************************************************************************************************
//
//		クラスの宣言
//
//********************************************************************************************************************
	//		net
	//****************************************************************************************************************
	class	NetPlayer	:	public	Player
	{
	private:
		int		m_id;

	public:
		NetPlayer( int id );
		void	Update( void );
	};

