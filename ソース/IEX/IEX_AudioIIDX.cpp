//*****************************************************************************************************************************
#include	"iextreme.h"

#include	"IEX_AudioIIDX.h"

//**************************************************************************************************************
//
//		サウンドバッファ
//
//**************************************************************************************************************

//**************************************************************************************************************
//
//**************************************************************************************************************
SoundBufferIIDX::SoundBufferIIDX(LPDIRECTSOUND lpDS, char* filename, bool b3D)
{
	DSBUFFERDESC	dsbd;
	LPVOID			lpbuf1, lpbuf2;
	DWORD			dwbuf1, dwbuf2;

	lpBuf3D = NULL;
	lpBuf = NULL;

	/*	WAVファイルのロード	*/
	lpWBuf = LoadWAV(filename, &size, &wfx);
	/*	ロード失敗	*/
	if (lpWBuf == NULL){
		return;
	}

	/* ファイル名保存 */
	sprintf_s(wav_file_path, sizeof(wav_file_path), "%s", filename);

	/* 二次バッファ作成	*/
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	if (b3D == true) dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRL3D;
	else dsbd.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = size;
	dsbd.lpwfxFormat = &wfx;
	if (lpDS->CreateSoundBuffer(&dsbd, &lpBuf, NULL) != DS_OK)
	{
		return;
	}

	lpBuf3D = NULL;
	if (b3D == true)
	{
		lpBuf->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID*)&lpBuf3D);
	}

	/* 二次バッファのロック	*/
	lpBuf->Lock(0, size, &lpbuf1, &dwbuf1, &lpbuf2, &dwbuf2, 0);
	/* 音源データの設定	*/
	memcpy(lpbuf1, lpWBuf, dwbuf1);
	if (dwbuf2 != 0) memcpy(lpbuf2, lpWBuf + dwbuf1, dwbuf2);
	/* 音源データの解放	*/
	GlobalFree(lpWBuf);
	/* 二次バッファのロック解除	*/
	lpBuf->Unlock(lpbuf1, dwbuf1, lpbuf2, dwbuf2);

	/* 自作初期化 */
	PlayCursor = 0;
	BufferSize = size;
	format = wfx;
}

SoundBufferIIDX::SoundBufferIIDX(LPDIRECTSOUND lpDS, SoundBufferIIDX *source)
{
	DSBUFFERDESC	dsbd;
	LPVOID			lpbuf1, lpbuf2;
	DWORD			dwbuf1, dwbuf2;

	lpBuf3D = NULL;
	lpBuf = NULL;

	/*	WAVファイルのロード	*/
	//lpWBuf = LoadWAV(source->wav_file_path, &size, &wfx);

	// 自力ごり押しコピー

	/* オープン	*/
	if ((hMMIO = mmioOpen(source->wav_file_path, NULL, MMIO_ALLOCBUF | MMIO_READ)) == NULL) return;

	/*	ｄａｔａチャンクに侵入		*/
	ckinfo.ckid = source->ckinfo.ckid;
	if (mmioGetInfo(hMMIO, &mminfo, 0) != 0) return;
	/*	バッファサイズ保存	*/
	size = source->size;
	/*	ＷＡＶ用バッファの取得	*/
	buf = (LPBYTE)GlobalAlloc(LPTR, size);
	if (buf == NULL) return;
	/*	データの読みとり	*/
	for (i = 0; i < size; i++){
		/*	エラーチェック	*/
		if (mminfo.pchNext >= mminfo.pchEndRead){
			if (mmioAdvance(hMMIO, &mminfo, MMIO_READ) != 0) return;
			//if (mminfo.pchNext >= mminfo.pchEndRead) return;
		}
		buf[i] = *((LPBYTE)mminfo.pchNext);
		mminfo.pchNext++;
	}
	/*	ファイルアクセス終了	*/
	//mmioSetInfo(hMMIO, &mminfo, 0);
	mmioClose(hMMIO, 0);

	BYTE receive[65535];
	memcpy_s(receive, sizeof(receive), source->lpWBuf, sizeof(source->lpWBuf));
	//memcpy_s(lpWBuf, sizeof(LPBYTE), source->lpWBuf, sizeof(source->lpWBuf));
	//lpWBuf = source->lpWBuf;
	//wfx = source->wfx;
	lpWBuf = buf;
	memcpy_s(&wfx, sizeof(wfx), &source->pwf, sizeof(source->pwf));


	/*	ロード失敗	*/
	if (lpWBuf == NULL){
		return;
	}

	/* 二次バッファ作成	*/
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	//if (b3D == true) dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRL3D;
	dsbd.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = size;
	dsbd.lpwfxFormat = &wfx;
	if (lpDS->CreateSoundBuffer(&dsbd, &lpBuf, NULL) != DS_OK)
	{
		return;
	}

	lpBuf3D = NULL;
	//if (b3D == true)
	//{
	//	lpBuf->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID*)&lpBuf3D);
	//}

	/* 二次バッファのロック	*/
	lpBuf->Lock(0, size, &lpbuf1, &dwbuf1, &lpbuf2, &dwbuf2, 0);
	/* 音源データの設定	*/
	memcpy(lpbuf1, lpWBuf, dwbuf1);
	if (dwbuf2 != 0) memcpy(lpbuf2, lpWBuf + dwbuf1, dwbuf2);
	/* 音源データの解放	*/
	GlobalFree(lpWBuf);
	/* 二次バッファのロック解除	*/
	lpBuf->Unlock(lpbuf1, dwbuf1, lpbuf2, dwbuf2);

	/* 自作初期化 */
	PlayCursor = 0;
	BufferSize = size;
	format = wfx;
}

void SoundBufferIIDX::Create_and_copy(LPDIRECTSOUND lpDS, char* filename, SoundBufferIIDX **buffers, int dst, int count)
{
	DSBUFFERDESC	dsbd;
	LPVOID			lpbuf1, lpbuf2;
	DWORD			dwbuf1, dwbuf2;


	//===========================================================
	//		コピー元作成
	//===========================================================
	buffers[dst]->lpBuf3D = NULL;
	buffers[dst]->lpBuf = NULL;

	/*	WAVファイルのロード	*/
	buffers[dst]->lpWBuf = buffers[dst]->LoadWAV(filename, &buffers[dst]->size, &buffers[dst]->wfx);

	/* ファイル名保存 */
	sprintf_s(buffers[dst]->wav_file_path, sizeof(buffers[dst]->wav_file_path), "%s", filename);
	/* 二次バッファ作成	*/
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = buffers[dst]->size;
	dsbd.lpwfxFormat = &buffers[dst]->wfx;
	if (lpDS->CreateSoundBuffer(&dsbd, &buffers[dst]->lpBuf, NULL) != DS_OK)
	{
		return;
	}

	/* 二次バッファのロック	*/
	buffers[dst]->lpBuf->Lock(0, buffers[dst]->size, &lpbuf1, &dwbuf1, &lpbuf2, &dwbuf2, 0);
	/* 音源データの設定	*/
	memcpy(lpbuf1, buffers[dst]->lpWBuf, dwbuf1);
	if (dwbuf2 != 0) memcpy(lpbuf2, buffers[dst]->lpWBuf + dwbuf1, dwbuf2);
	/* 音源データの解放	*/
	//GlobalFree(lpWBuf);	// この文をコメントアウトするのに全てが詰まってる
	/* 二次バッファのロック解除	*/
	buffers[dst]->lpBuf->Unlock(lpbuf1, dwbuf1, lpbuf2, dwbuf2);

	/* 自作初期化 */
	buffers[dst]->PlayCursor = 0;
	buffers[dst]->BufferSize = buffers[dst]->size;
	buffers[dst]->format = buffers[dst]->wfx;


	//===========================================================
	//		コピー先作成
	//===========================================================
	for (int no = dst + 1; no < dst + count; no++)
	{

		DSBUFFERDESC	dsbd;
		LPVOID			lpbuf1 = NULL, lpbuf2 = NULL;
		DWORD			dwbuf1 = 0, dwbuf2 = 0;

		buffers[no]->lpBuf3D = NULL;
		buffers[no]->lpBuf = NULL;

		/* 二次バッファ作成	*/
		ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
		dsbd.dwSize = sizeof(DSBUFFERDESC);
		//if (b3D == true) dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRL3D;
		dsbd.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME;
		dsbd.dwBufferBytes = buffers[dst]->size;
		dsbd.lpwfxFormat = &buffers[dst]->wfx;

		HRESULT result = lpDS->CreateSoundBuffer(&dsbd, &buffers[no]->lpBuf, NULL);
		if (result != DS_OK)
		{
			return;
		}

		/* 二次バッファのロック	*/
		buffers[no]->lpBuf->Lock(0, buffers[dst]->size, &lpbuf1, &dwbuf1, &lpbuf2, &dwbuf2, 0);
		/* 音源データの設定	*/
		memcpy(lpbuf1, buffers[dst]->lpWBuf, dwbuf1);
		if (dwbuf2 != 0) memcpy(lpbuf2, buffers[dst]->lpWBuf + dwbuf1, dwbuf2);
		/* 音源データの解放	*/
		//GlobalFree(buffers[no].lpWBuf);
		/* 二次バッファのロック解除	*/
		buffers[no]->lpBuf->Unlock(lpbuf1, dwbuf1, lpbuf2, dwbuf2);

		/* 自作初期化 */
		buffers[no]->PlayCursor = 0;
		buffers[no]->BufferSize = buffers[dst]->size;
		buffers[no]->format = buffers[dst]->wfx;
	}

	/* コピー元音源データの解放	*/
	GlobalFree(buffers[dst]->lpWBuf);
}

SoundBufferIIDX::~SoundBufferIIDX()
{
	if (lpBuf != NULL)lpBuf->Release();
	if (lpBuf3D != NULL) lpBuf3D->Release();

}

//**************************************************************************************************************
//		ＷＡＶファイルの読み込み
//**************************************************************************************************************
LPBYTE SoundBufferIIDX::LoadWAV(LPSTR fname, LPDWORD size, LPWAVEFORMATEX wfx)
{
	/* オープン	*/
	if ((hMMIO = mmioOpen(fname, NULL, MMIO_ALLOCBUF | MMIO_READ)) == NULL) return NULL;
	if (mmioDescend(hMMIO, &ckparent, NULL, 0) != 0) goto WAVE_LoadError;
	/*	ＷＡＶ(RIFF)ファイルチェック		*/
	if ((ckparent.ckid != FOURCC_RIFF) || (ckparent.fccType != mmioFOURCC('W', 'A', 'V', 'E'))) goto WAVE_LoadError;
	/*	ｆｍｔチャンクに侵入		*/
	ckinfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(hMMIO, &ckinfo, &ckparent, MMIO_FINDCHUNK) != 0) goto WAVE_LoadError;
	if (ckinfo.cksize < sizeof(PCMWAVEFORMAT)) goto WAVE_LoadError;
	/*	チャンクからリード	*/
	if (mmioRead(hMMIO, (HPSTR)&pwf, sizeof(pwf)) != sizeof(pwf)) goto WAVE_LoadError;
	if (pwf.wf.wFormatTag != WAVE_FORMAT_PCM) goto WAVE_LoadError;
	/*	WAVフォーマットの保存	*/
	memset(wfx, 0, sizeof(WAVEFORMATEX));
	memcpy(wfx, &pwf, sizeof(pwf));
	/*	データの読み込み	*/
	if (mmioSeek(hMMIO, ckparent.dwDataOffset + sizeof(FOURCC), SEEK_SET) == -1) goto WAVE_LoadError;
	/*	ｄａｔａチャンクに侵入		*/
	ckinfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hMMIO, &ckinfo, &ckparent, MMIO_FINDCHUNK) != 0) goto WAVE_LoadError;
	if (mmioGetInfo(hMMIO, &mminfo, 0) != 0) goto WAVE_LoadError;
	/*	バッファサイズ保存	*/
	if (size != NULL) *size = ckinfo.cksize;
	/*	ＷＡＶ用バッファの取得	*/
	buf = (LPBYTE)GlobalAlloc(LPTR, ckinfo.cksize);
	if (buf == NULL) goto WAVE_LoadError;
	/*	データの読みとり	*/
	for (i = 0; i < ckinfo.cksize; i++){
		/*	エラーチェック	*/
		if (mminfo.pchNext >= mminfo.pchEndRead){
			if (mmioAdvance(hMMIO, &mminfo, MMIO_READ) != 0) goto WAVE_LoadError;
			if (mminfo.pchNext >= mminfo.pchEndRead) goto WAVE_LoadError;
		}
		*(buf + i) = *((LPBYTE)mminfo.pchNext);
		mminfo.pchNext++;
	}
	/*	ファイルアクセス終了	*/
	mmioSetInfo(hMMIO, &mminfo, 0);
	mmioClose(hMMIO, 0);
	return buf;

WAVE_LoadError:	/*	エラー終了	*/
	mmioClose(hMMIO, 0);
	if (buf != NULL) GlobalFree(buf);
	return NULL;
}


//**************************************************************************************************************
//	情報設定
//**************************************************************************************************************
//-------------------------------------------------------------
//	位置設定	
//-------------------------------------------------------------

//**************************************************************************************************************
//	再生管理
//**************************************************************************************************************
//-------------------------------------------------------------
//	再生	
//-------------------------------------------------------------
void SoundBufferIIDX::Play(BOOL loop)
{
	lpBuf->Stop();
	lpBuf->SetCurrentPosition(PlayCursor);
	//	ループ再生
	if (loop) lpBuf->Play(0, 0, DSBPLAY_LOOPING);
	//	ノーマル再生
	else	   lpBuf->Play(0, 0, 0);

	//	lpBuf->SetFrequency(8000);


	PlayCursor = 0;
}

//-------------------------------------------------------------
//	停止	
//-------------------------------------------------------------
void SoundBufferIIDX::Stop()
{
	lpBuf->Stop();
}

void SoundBufferIIDX::Pause()
{
	lpBuf->GetCurrentPosition(&PlayCursor, NULL);
	lpBuf->Stop();
}

//-------------------------------------------------------------
//	ボリューム変更
//-------------------------------------------------------------
void SoundBufferIIDX::SetVolume(int volume)
{
	lpBuf->SetVolume(volume);
}

//-------------------------------------------------------------
//	ボリュームゲッター
//-------------------------------------------------------------
int	SoundBufferIIDX::GetVolume()
{
	LONG ret;
	HRESULT a = lpBuf->GetVolume(&ret);
	return ret;
}

//-------------------------------------------------------------
//	ステレオ(左右音)関係
//-------------------------------------------------------------
void SoundBufferIIDX::SetPan(int pan)
{
	lpBuf->SetPan(pan);
}

int SoundBufferIIDX::GetPan()
{
	LONG ret;
	lpBuf->GetPan(&ret);
	return ret;
}


//-------------------------------------------------------------
//	周波数関係(再生速度・ピッチ)
//-------------------------------------------------------------
void SoundBufferIIDX::SetFrequency(int pitch)
{
	HRESULT a = lpBuf->SetFrequency(pitch);
}

int SoundBufferIIDX::GetFrequency()
{
	DWORD ret;
	lpBuf->GetFrequency(&ret);
	return ret;
}


//-------------------------------------------------------------
//	再生チェック	
//-------------------------------------------------------------
BOOL SoundBufferIIDX::isPlay()
{
	DWORD	dwAns;
	lpBuf->GetStatus(&dwAns);
	if ((dwAns&DSBSTATUS_PLAYING) != 0) return TRUE;
	else return FALSE;
}


//-------------------------------------------------------------
//	再生位置関係
//-------------------------------------------------------------
DWORD SoundBufferIIDX::GetPlayCursor()
{
	DWORD ret;
	lpBuf->GetCurrentPosition(&ret, NULL);

	return ret;
}

DWORD SoundBufferIIDX::GetPlayFrame()
{
	return (GetPlayCursor() / (format.nAvgBytesPerSec / 60));
}

int SoundBufferIIDX::GetPlaySecond()
{
	return (GetPlayCursor() / format.nAvgBytesPerSec);
}

void SoundBufferIIDX::SetPlaySecond(int sec)
{
	DWORD set = sec * format.nAvgBytesPerSec;
	lpBuf->SetCurrentPosition(set);
}

DWORD SoundBufferIIDX::GetSize()
{
	return BufferSize;
}

int SoundBufferIIDX::GetLengthSecond()
{
	return (BufferSize / format.nAvgBytesPerSec);
}

//**************************************************************************************************************
//
//		ストリームサウンド
//
//**************************************************************************************************************

static int	NumStream = 0;
static bool ForceEnd = false;

//**************************************************************************************************************
//		管理スレッド
//**************************************************************************************************************
DWORD WINAPI ThreadIIDX(LPDWORD lpdwParam)
{
	DWORD	param;
	iexStreamSoundIIDX*	lpStream;

	lpStream = (iexStreamSoundIIDX*)(lpdwParam);
	for (;;){
		if (lpStream == NULL) break;
		param = WaitForMultipleObjects(3, lpStream->hEvent, FALSE, 100);

		if (ForceEnd == true) param = -1;

		switch (param){
		case WAIT_OBJECT_0:		if (lpStream->type == TYPE_WAV) lpStream->SetBlockWAV(1);
			if (lpStream->type == TYPE_OGG) lpStream->SetBlockOGG(1);
			break;
		case WAIT_OBJECT_0 + 1:	if (lpStream->type == TYPE_WAV) lpStream->SetBlockWAV(0);
			if (lpStream->type == TYPE_OGG) lpStream->SetBlockOGG(0);
			break;

		case WAIT_TIMEOUT:		switch (lpStream->GetMode()){
		case STR_NORMAL:		break;
		case STR_FADEIN:	lpStream->SetVolume(lpStream->GetVolume() + lpStream->GetParam());
			if (lpStream->GetVolume() >= 255) lpStream->SetMode(STR_NORMAL, 0);
			break;
		case STR_FADEOUT:	lpStream->SetVolume(lpStream->GetVolume() - lpStream->GetParam());
			if (lpStream->GetVolume() <= 0){
				lpStream->Stop();
				delete lpStream;
				ExitThread(TRUE);
				return 0;
			}
			break;
		}
								break;

		default:
			lpStream->Stop();
			delete lpStream;
			ExitThread(TRUE);
			return 0;

		}
	}
	return 0;
}

//**************************************************************************************************************
//
//**************************************************************************************************************
iexStreamSoundIIDX::iexStreamSoundIIDX(LPDIRECTSOUND lpDS, LPSTR filename, BYTE mode, int param)
{
	NumStream++;

	LPSTR	work;
	BOOL	bInit;
	//	ストリームファイルを開く
	work = &filename[lstrlen(filename) - 4];
	if (lstrcmpi(work, ".wav") == 0) bInit = SetWAV(lpDS, filename);
	if (lstrcmpi(work, ".ogg") == 0) bInit = SetOGG(lpDS, filename);

	lpStream->SetCurrentPosition(0);
	if (mode != STR_FADEIN) SetVolume(255);
	else SetVolume(0);
	/*	管理スレッドの作成	*/
	hStrThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadIIDX, this, 0, &dwThreadId);
	if (hStrThread == NULL) return;
	/*	再生開始	*/
	lpStream->Play(0, 0, DSBPLAY_LOOPING);

	this->mode = mode;
	this->param = param;
}



iexStreamSoundIIDX::~iexStreamSoundIIDX()
{
	if (lpStream != NULL){
		if (type == TYPE_OGG) ov_clear(&vf);
		else if (hStrFile != NULL) fclose(hStrFile);

		if (lpStrNotify != NULL) lpStrNotify->Release();
		lpStrNotify = NULL;
		/*	バッファ開放	*/
		if (lpStream != NULL) lpStream->Release();
		lpStream = NULL;
	}

	NumStream--;
}

//**************************************************************************************************************
//
//**************************************************************************************************************
//-------------------------------------------------------------
//	ブロック作成
//-------------------------------------------------------------
BOOL	iexStreamSoundIIDX::OGGRead(LPBYTE dst, unsigned long size)
{
	DWORD	remain = size;
	char*	dstPtr = (char*)dst;
	while (remain > 0){
		long actualRead = ov_read(&vf, dstPtr, remain, 0, 2, 1, NULL);
		//終端チェック
		if (actualRead <= 0){
			if (ov_pcm_seek(&vf, 0)) return FALSE;
		}
		remain -= actualRead;
		dstPtr += actualRead;
	}
	return TRUE;
}

BOOL	iexStreamSoundIIDX::SetBlockOGG(int block)
{
	LPBYTE	blk1, blk2;
	DWORD	bs1, bs2;

	DWORD	CurPos;

	CurPos = StrSize - StrPos;
	//	バッファロック
	lpStream->Lock((rate * 4 * STRSECOND)*block, rate * 4 * STRSECOND, (LPVOID*)&blk1, &bs1, (LPVOID*)&blk2, &bs2, 0);
	//	ブロック１へのデータセット
	OGGRead(blk1, bs1);
	StrPos += bs1;
	//	ブロック２へのデータセット
	if (blk2){
		OGGRead(blk2, bs2);
		StrPos += bs2;
	}

	lpStream->Unlock(blk1, bs1, blk2, bs2);
	return TRUE;
}


BOOL	iexStreamSoundIIDX::SetBlockWAV(int block)
{
	LPBYTE	blk1, blk2;
	DWORD	bs1, bs2, work;

	DWORD	CurPos;

	CurPos = StrSize - StrPos;
	lpStream->Lock((rate * 4 * STRSECOND)*block, rate * 4 * STRSECOND, (LPVOID*)&blk1, &bs1, (LPVOID*)&blk2, &bs2, 0);

	work = fread(blk1, bs1, 1, hStrFile);
	StrPos += work;
	if (work < bs1){
		/*	巻き戻し	*/
		fseek(hStrFile, LoopPtr, SEEK_SET);
		work = fread(blk1 + work, bs1 - work, 1, hStrFile);
		StrPos = work;
	}

	if (blk2){
		work = fread(blk2, bs2, 1, hStrFile);
		StrPos += work;
		if (work < bs2){
			/*	巻き戻し	*/
			fseek(hStrFile, LoopPtr, SEEK_SET);
			work = fread(blk2 + work, bs2 - work, 1, hStrFile);
			StrPos = work;
		}
	}

	lpStream->Unlock(blk1, bs1, blk2, bs2);
	return TRUE;
}


//**************************************************************************************************************
//	データ管理
//**************************************************************************************************************
void iexStreamSoundIIDX::Initialize(LPDIRECTSOUND lpDS, int rate)
{
	DSBUFFERDESC	dsbd;
	WAVEFORMATEX	wfx;


	/*	初期化チェック	*/
	if (lpDS == NULL) return;
	/*	ＷＡＶＥフォーマット初期化	*/
	ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;			/*	stereo	*/
	wfx.nSamplesPerSec = rate;
	wfx.wBitsPerSample = 16;			/*	16bit	*/
	wfx.nBlockAlign = 4;
	wfx.nAvgBytesPerSec = rate * 4;
	/* 二次バッファの初期化	*/
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_LOCSOFTWARE;
	dsbd.dwBufferBytes = rate * 4 * STRSECOND * 2;
	dsbd.lpwfxFormat = &wfx;
	if (lpDS->CreateSoundBuffer(&dsbd, &lpStream, NULL) != DS_OK) return;
	lpStream->SetFormat(&wfx);

	if (lpStream->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&lpStrNotify) != DS_OK) return;
	/*	位置イベント作成	*/
	hEvent[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	hEvent[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
	hEvent[2] = CreateEvent(NULL, FALSE, FALSE, NULL);

	pn[0].dwOffset = 0;
	pn[0].hEventNotify = hEvent[0];
	pn[1].dwOffset = rate * 4 * STRSECOND;
	pn[1].hEventNotify = hEvent[1];

	pn[2].dwOffset = DSBPN_OFFSETSTOP;
	pn[2].hEventNotify = hEvent[2];

	if (lpStrNotify->SetNotificationPositions(3, pn) != DS_OK) return;

	this->rate = rate;
	return;
}


BOOL	iexStreamSoundIIDX::SetWAV(LPDIRECTSOUND lpDS, char* filename)
{
	HMMIO			hMMIO = NULL;		/*	ファイルハンドル	*/
	MMCKINFO		ckinfo, ckparent;	/*	RIFFチャンク情報	*/
	LRESULT			ptr;

	/* オープン	*/
	hMMIO = mmioOpen(filename, NULL, MMIO_ALLOCBUF | MMIO_READ);
	mmioDescend(hMMIO, &ckparent, NULL, 0);
	/*	ｄａｔａチャンクに侵入		*/
	ckinfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
	mmioDescend(hMMIO, &ckinfo, &ckparent, MMIO_FINDCHUNK);
	//	現在位置取得
	ptr = mmioSeek(hMMIO, 0, SEEK_CUR);
	/*	ファイルアクセス終了	*/
	mmioClose(hMMIO, 0);
	if (ptr == -1) return FALSE;

	/*	ファイルオープン	*/
	hStrFile = fopen(filename, "rb");
	if (hStrFile == NULL) return FALSE;
	//	ストリーム情報設定
	StrPos = 0;
	LoopPtr = ptr;

	fseek(hStrFile, 0L, SEEK_END);
	StrSize = ftell(hStrFile) - LoopPtr;

	StrSize = GetFileSize(hStrFile, NULL) - LoopPtr;
	//	ファイルシーク
	fseek(hStrFile, LoopPtr, SEEK_SET);

	type = TYPE_WAV;
	Initialize(lpDS, 22050);

	/*	再生準備	*/
	SetBlockWAV(0);

	return TRUE;
}

//
//	OGG用ストリーム初期化
//

BOOL	iexStreamSoundIIDX::SetOGG(LPDIRECTSOUND lpDS, char* filename)
{
	//	ファイルオープン 
	hStrFile = fopen(filename, "rb");
	if (hStrFile == NULL) return FALSE;

	//Oggを開く
	ov_open(hStrFile, &vf, NULL, 0);

	//シーク可能?
	if (!ov_seekable(&vf)){
		ov_clear(&vf);
		return FALSE;
	}

	//情報設定
	const vorbis_info *info = ov_info(&vf, -1);
	Initialize(lpDS, info->rate);

	type = TYPE_OGG;
	StrPos = 0;
	StrSize = (DWORD)ov_pcm_total(&vf, -1);

	/*	再生準備	*/
	SetBlockOGG(0);

	return TRUE;
}


//**************************************************************************************************************
//	再生管理
//**************************************************************************************************************

void iexStreamSoundIIDX::Stop()
{
	if (lpStream == NULL) return;
	if (hStrFile == NULL) return;

	lpStream->Stop();
}

void iexStreamSoundIIDX::SetVolume(int volume)
{
	int		vol;
	if (lpStream == NULL) return;
	/*	音量セット	*/
	if (volume > 255) volume = 255;
	if (volume < 0) volume = 0;
	this->volume = volume;
	volume -= 255;
	vol = (volume*volume * 100 / (255 * 255));
	lpStream->SetVolume(-vol*vol);
}

void iexStreamSoundIIDX::SetMode(BYTE mode, int param)
{
	this->mode = mode;
	this->param = param;
}




//**************************************************************************************************************
//
//		サウンドマネージャ
//
//**************************************************************************************************************

//**************************************************************************************************************
//
//**************************************************************************************************************
iexSoundIIDX::iexSoundIIDX()
{
	hWndWAV = iexSystem::Window;
	CoInitialize(NULL);
	//	DirectSoundの初期化
	if (DirectSoundCreate8(NULL, &lpDS, NULL) != DS_OK){
		lpDS = NULL;
		return;
	}

	lpDS->SetCooperativeLevel(hWndWAV, DSSCL_PRIORITY);
	for (int i = 0; i<WavNum; i++) buffer[i] = NULL;

	lpPrimary = NULL;
	DSBUFFERDESC	dsbd;
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
	lpDS->CreateSoundBuffer(&dsbd, &lpPrimary, NULL);

	lp3DListener = NULL;
	lpPrimary->QueryInterface(IID_IDirectSound3DListener, (LPVOID *)lp3DListener);

	WAVEFORMATEX   wfx;
	ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = 22050;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	lpPrimary->SetFormat(&wfx);

}

iexSoundIIDX::~iexSoundIIDX()
{
	int		i;

	//	バッファの解放
	for (i = 0; i<WavNum; i++){
		if (buffer[i] != NULL) delete buffer[i];
		buffer[i] = NULL;
	}

	//	Direct Sound解放
	if (lpPrimary != NULL) lpPrimary->Release();
	if (lpDS != NULL) lpDS->Release();

	lpDS = NULL;
	lpPrimary = NULL;
}

//**************************************************************************************************************
//
//**************************************************************************************************************
void iexSoundIIDX::Set(int no, char* filename, bool b3D)
{
	//	初期化チェック
	if (lpDS == NULL) return;
	//	既存のバッファの解放
	if (buffer[no] != NULL) delete buffer[no];
	buffer[no] = NULL;
	//	WAVファイルのロード
	if (filename != NULL)
	{
		buffer[no] = new SoundBufferIIDX(lpDS, filename, b3D);
		if (buffer[no]->GetBuf() == NULL)
		{
			delete buffer[no];
			buffer[no] = NULL;
		}
	}
}

void iexSoundIIDX::Set_copy(int set_no, int source_no)
{
	//	初期化チェック
	if (lpDS == NULL) return;
	//	既存のバッファの解放
	if (buffer[set_no] != NULL) delete buffer[set_no];
	buffer[set_no] = NULL;
	//	WAVファイルのロード
	if (buffer[source_no] != NULL)
	{
		buffer[set_no] = new SoundBufferIIDX(lpDS, buffer[source_no]);
		if (buffer[set_no]->GetBuf() == NULL)
		{
			delete buffer[set_no];
			buffer[set_no] = NULL;
		}
	}
}

void iexSoundIIDX::Set_copy(char *filename, int dst, int count)
{
	if (count == 1)
	{
		Set(dst, filename);	// 1個しかないならコピーする意味ないじゃない！
		return;
	}

	//	初期化チェック
	if (lpDS == NULL) return;

	for (int i = dst; i < dst + count; i++)
	{
		//	既存のバッファの解放
		if (buffer[i] != NULL) delete buffer[i];
		buffer[i] = NULL;

		buffer[i] = new SoundBufferIIDX();
	}

	//	WAVファイルのロード
	if (filename != NULL)
	{
		SoundBufferIIDX::Create_and_copy(lpDS, filename, buffer, dst, count);
		//if (buffer[set_no]->GetBuf() == NULL)
		//{
		//	delete buffer[set_no];
		//	buffer[set_no] = NULL;
		//}
	}
}

//**************************************************************************************************************
//
//**************************************************************************************************************

void iexSoundIIDX::Play(int no, BOOL loop)
{
	//	初期化チェック
	if (lpDS == NULL) return;
	//	データが無い！！
	if (buffer[no] == NULL) return;

	buffer[no]->Play(loop);
}

void iexSoundIIDX::Stop(int no)
{
	//	初期化チェック
	if (lpDS == NULL) return;
	//	データが無い！！
	if (buffer[no] == NULL) return;

	buffer[no]->Stop();
}

void iexSoundIIDX::Pause(int no)
{
	//	初期化チェック
	if (lpDS == NULL) return;
	//	データが無い！！
	if (buffer[no] == NULL) return;

	buffer[no]->Pause();
}


/*	ボリュームの設定	*/
void	iexSoundIIDX::SetVolume(int no, int volume)
{
	//	初期化チェック
	if (lpDS == NULL) return;
	//	データが無い！！
	if (buffer[no] == NULL) return;
	//	音量セット(0〜-10000の範囲)
	buffer[no]->SetVolume(volume);
}

int		iexSoundIIDX::GetVolume(int no)
{
	//	初期化チェック
	if (lpDS == NULL) return 0;
	//	データが無い！！
	if (buffer[no] == NULL) return 0;
	//	音量ゲット(0〜-10000の範囲)
	return buffer[no]->GetVolume();
}

/*	パン(ステレオ)の設定	*/
void	iexSoundIIDX::SetPan(int no, int pan)
{
	//	初期化チェック
	if (lpDS == NULL) return;
	//	データが無い！！
	if (buffer[no] == NULL) return;
	//	パンセット(-10000(左)〜0(真ん中)〜10000(右)の範囲)
	buffer[no]->SetPan(pan);
}
int		iexSoundIIDX::GetPan(int no)
{
	//	初期化チェック
	if (lpDS == NULL) return 0;
	//	データが無い！！
	if (buffer[no] == NULL) return 0;
	//	現在のパンを返す
	return buffer[no]->GetPan();
}

/*	周波数の設定	*/
void	iexSoundIIDX::SetFrequency(int no, int pitch)
{
	//	初期化チェック
	if (lpDS == NULL) return;
	//	データが無い！！
	if (buffer[no] == NULL) return;
	//	セット
	buffer[no]->SetFrequency(pitch);
}
int		iexSoundIIDX::GetFrequency(int no)
{
	//	初期化チェック
	if (lpDS == NULL) return 0;
	//	データが無い！！
	if (buffer[no] == NULL) return 0;
	//	セット
	return buffer[no]->GetFrequency();
}


/*	再生状況のチェック	*/
BOOL	iexSoundIIDX::isPlay(int no)
{
	//	初期化チェック
	if (lpDS == NULL) return FALSE;
	//	データが無い！！
	if (buffer[no] == NULL) return FALSE;

	return buffer[no]->isPlay();
}

/*	pos(再生位置)の取得	*/
DWORD	iexSoundIIDX::GetPlayCursor(int no)
{
	//	初期化チェック
	if (lpDS == NULL) return 0;
	//	データが無い！！
	if (buffer[no] == NULL) return 0;

	return buffer[no]->GetPlayCursor();
}
DWORD	iexSoundIIDX::GetPlayFrame(int no)
{
	//	初期化チェック
	if (lpDS == NULL) return 0;
	//	データが無い！！
	if (buffer[no] == NULL) return 0;

	return buffer[no]->GetPlayFrame();
}
int		iexSoundIIDX::GetPlaySecond(int no)
{
	//	初期化チェック
	if (lpDS == NULL) return 0;
	//	データが無い！！
	if (buffer[no] == NULL) return 0;

	return buffer[no]->GetPlaySecond();
}

void	iexSoundIIDX::SetPlaySecond(int no, int sec)
{
	//	初期化チェック
	if (lpDS == NULL) return;
	//	データが無い！！
	if (buffer[no] == NULL) return;

	buffer[no]->SetPlaySecond(sec);
}


DWORD iexSoundIIDX::GetSize(int no)
{
	//	初期化チェック
	if (lpDS == NULL) return 0;
	//	データが無い！！
	if (buffer[no] == NULL) return 0;

	return buffer[no]->GetSize();
}

int		iexSoundIIDX::GetLengthSecond(int no)
{
	//	初期化チェック
	if (lpDS == NULL) return 0;
	//	データが無い！！
	if (buffer[no] == NULL) return 0;

	return buffer[no]->GetLengthSecond();
}

//**************************************************************************************************************
//	ストリームサウンド管理
//**************************************************************************************************************
iexStreamSoundIIDX* iexSoundIIDX::PlayStream(char* filename)
{
	return PlayStream(filename, STR_NORMAL, 0);
}

iexStreamSoundIIDX* iexSoundIIDX::PlayStream(char* filename, BYTE mode, int param)
{
	iexStreamSoundIIDX*	lpStream;

	//	初期化チェック
	if (lpDS == NULL) return NULL;

	lpStream = new iexStreamSoundIIDX(lpDS, filename, mode, param);
	return lpStream;
}

