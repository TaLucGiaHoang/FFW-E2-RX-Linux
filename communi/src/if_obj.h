///////////////////////////////////////////////////////////////////////////////
/**
 * @file  if_obj.h
 * @brief Communi.dll Header File for Base Class
 * @author M.Yamamoto(EM2)
 * @author Copyright(C) 2015 Renesas Electronics Corporation
 * @author and Renesas System Design Corporation All rights reserved.
 * @date 2015/01/15
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴 (V.3.00.00.000から記載)
・RevRxE2LNo141104-001 2015/01/15 山本
	新規作成
*/
#ifndef	__IF_OBH_H__
#define	__IF_OBH_H__

#ifndef WINVER
#define WINVER 0x0600	// カーネルベースとして、メジャー番号0x600の最低値(Windows Vista)と定義する。
#endif

#if (0)	// Windows (1), Linux (0)
#include	<afx.h>	// WINDOWS
#else
#include "afx_linux.h"	// LINUX
#endif

//-----------------------------------------------------------------------------
/// Communi.DLL Interface Base Class
//-----------------------------------------------------------------------------
class CIF_OBJ
{
public:
			CIF_OBJ()			// Constructor
			{
				com_err = 0;		// 通信エラーを”エラーなし”に初期化
				com_type = 0;		// 通信タイプを初期化(未定義値)
				TimeOut = 4 * 1000;	// 通信タイムアウトを4secに初期化
				p_size = 0;			// 受信データバイト数を0に初期化
			}
	virtual	~CIF_OBJ(){}			// Virual Destructor。何もしない。

	// Member Method (Public) Warning除去のため、メンバ関数はすべて仮引数とする。
	virtual	int		Open(long , char*, long )
					{
						return FALSE;
					}
	virtual	int		Close(void)
					{
						return FALSE;
					}
	virtual	int		SendBlock(char*, long )
					{
						return FALSE;
					}
	virtual	int		ReceiveBlock(char*, long )
					{
						return FALSE;
					}
	virtual	int		ReceiveBlockH(char*, long*)
					{
						return FALSE;
					}
	virtual	int		ReceiveBlockM(char*, long)
					{
						return FALSE;
					}
	virtual	int		ReceivePolling(long *)
					{
						return FALSE;
					}
	virtual	int		SetTimeOut(long )
					{
						return FALSE;
					}
	virtual	int		SetTransferSize(long )
					{
						return FALSE;
					}
	virtual int		Lock(void)
					{
						return TRUE;
					}
	virtual int		UnLock(void)
					{
						return TRUE;
					}
	virtual int		VendorControlIn(char, unsigned short, unsigned short, char *, int)
					{
						return TRUE;
					}
	virtual int		VendorControlOut(char, unsigned short, unsigned short, const char *, int)
					{
						return TRUE;
					}
			int		Return_data_size(int *size)
					{
						*size = p_size;
						return TRUE;
					}
			int		GetLastError(void)
					{
						return com_err;
					}
			void	SetErr(int err)
					{
						com_err = err;
					}
			int		GetComType(void)
					{
						return com_type;
					}
protected:
			// Member Variable
			long	TimeOut;		// 通信タイムアウト時間
			int		p_size;			// 受信済みデータ数(ReceiveBlock専用。ReceiveBlockH,ReceiveBlockMは対象外)
			int		com_type;		// 通信インターフェース
private:
			int		com_err;		// エラー番号
};

#endif
