
#pragma once

class Timer
{
public:
	Timer();
	~Timer();

	// 計測開始
	void Start();
	// 制限時間<秒 * 1000>
	void Start(int limit);
	void Start(int m, int s, int ms);
	// 初期化
	void Reset();
	// 経過時間計算
	int Check();

	// 経過時間取得
	int Get_delta();
	float Get_second();
	void Get_second(int *s,	// 秒
		int *ms);	// ミリ秒
	void Get_second(int *m,	// 分
		int *s,		// 秒
		int *ms);	// ミリ秒

	// 残り時間取得
	int Get_delta_limit();
	float Get_second_limit();
	void Get_second_limit(int *s,	// 秒
		int *ms);		// ミリ秒
	void Get_second_limit(int *m,	// 分
		int *s,			// 秒
		int *ms);		// ミリ秒


private:
	int start; // 計測開始時間
	int delta; // 経過時間<秒/1000>
	int limit; // 制限時間
};

#define LIMIT_TIME 120 + 6	// +6は、よーいドンの補間extern Timer *timer;
extern Timer *timer;