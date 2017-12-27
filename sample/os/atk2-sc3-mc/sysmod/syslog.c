/*
 *  TOPPERS ATK2
 *      Toyohashi Open Platform for Embedded Real-Time Systems
 *      Automotive Kernel Version 2
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2005-2015 by Center for Embedded Computing Systems
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2011-2015 by FUJI SOFT INCORPORATED, JAPAN
 *  Copyright (C) 2011-2013 by Spansion LLC, USA
 *  Copyright (C) 2011-2015 by NEC Communication Systems, Ltd., JAPAN
 *  Copyright (C) 2011-2015 by Panasonic Advanced Technology Development Co., Ltd., JAPAN
 *  Copyright (C) 2011-2014 by Renesas Electronics Corporation, JAPAN
 *  Copyright (C) 2011-2015 by Sunny Giken Inc., JAPAN
 *  Copyright (C) 2011-2015 by TOSHIBA CORPORATION, JAPAN
 *  Copyright (C) 2011-2015 by Witz Corporation
 *  Copyright (C) 2014-2015 by AISIN COMCRUISE Co., Ltd., JAPAN
 *  Copyright (C) 2014-2015 by eSOL Co.,Ltd., JAPAN
 *  Copyright (C) 2014-2015 by SCSK Corporation, JAPAN
 *  Copyright (C) 2015 by SUZUKI MOTOR CORPORATION
 *
 *  上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 *
 *  本ソフトウェアは，AUTOSAR（AUTomotive Open System ARchitecture）仕
 *  様に基づいている．上記の許諾は，AUTOSARの知的財産権を許諾するもので
 *  はない．AUTOSARは，AUTOSAR仕様に基づいたソフトウェアを商用目的で利
 *  用する者に対して，AUTOSARパートナーになることを求めている．
 *
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 *
 *  $Id: syslog.c 425 2015-12-07 08:06:19Z witz-itoyo $
 */

/*
 *		システムログ機能
 */


/*
 *  AUTOSAR共通のデータ型・定数・マクロ
 */
#include "Std_Types.h"

#include "t_syslog.h"
#include "log_output.h"
#include "target_sysmod.h"
#include "syslog.h"

/*
 *  トレースログマクロのデフォルト定義
 */
#ifndef LOG_SYSLOG_WRI_LOG_ENTER
#define LOG_SYSLOG_WRI_LOG_ENTER(prio, p_syslog)
#endif /* LOG_SYSLOG_WRI_LOG_ENTER */

#ifndef LOG_SYSLOG_WRI_LOG_LEAVE
#define LOG_SYSLOG_WRI_LOG_LEAVE(ercd)
#endif /* LOG_SYSLOG_WRI_LOG_LEAVE */

#ifndef LOG_SYSLOG_MSK_LOG_ENTER
#define LOG_SYSLOG_MSK_LOG_ENTER(lowmask)
#endif /* LOG_SYSLOG_MSK_LOG_ENTER */

#ifndef LOG_SYSLOG_MSK_LOG_LEAVE
#define LOG_SYSLOG_MSK_LOG_LEAVE(ercd)
#endif /* LOG_SYSLOG_MSK_LOG_LEAVE */

/*
 *  出力すべきログ情報の重要度（ビットマップ）
 */
static uint32 syslog_lowmask_not;      /* 低レベル出力すべき重要度（反転）*/

/*
 *  システムログ機能の初期化
 */
void
_syslog_syslog_initialize(void)
{
	syslog_lowmask_not = 0U;
}

/*
 *  ログ情報の出力
 *
 *  CPUロック状態や実行コンテキストによらず動作できるように実装してある
 */
StatusType
_syslog_syslog_wri_log(uint32 prio, const SYSLOG *p_syslog)
{
	SIL_PRE_LOC;
	LOG_SYSLOG_WRI_LOG_ENTER(prio, p_syslog);

#ifdef G_SYSLOG
	SIL_LOC_SPN();
#else
	SIL_LOC_INT();
#endif /* G_SYSLOG */

	/*
	 *  低レベル出力
	 */
	if (((~syslog_lowmask_not) & LOG_MASK(prio)) != 0U) {
		_syslog_syslog_print(p_syslog, &target_fput_log);
		(*target_fput_log)('\n');
	}

#ifdef G_SYSLOG
	SIL_UNL_SPN();
#else
	SIL_UNL_INT();
#endif /* G_SYSLOG */

	LOG_SYSLOG_WRI_LOG_LEAVE(E_OK);
	return(E_OK);
}

/*
 *  出力すべきログ情報の重要度の設定
 */
StatusType
_syslog_syslog_msk_log(uint32 lowmask)
{
	LOG_SYSLOG_MSK_LOG_ENTER(lowmask);
	syslog_lowmask_not = ~lowmask;
	LOG_SYSLOG_MSK_LOG_LEAVE(E_OK);
	return(E_OK);
}


void
syslog_initialize(void)
{
	KERLIBSLG_PRM prm;

	prm.index = KERLIBSLG_ID_SYSLOG_INITIALIZE;
	KernelLibrarySyslog(&prm);
	return;
}

StatusType
syslog_wri_log(uint32 prio, const SYSLOG *p_syslog)
{
	KERLIBSLG_PRM prm;

	prm.index = KERLIBSLG_ID_SYSLOG_WRI_LOG;
	prm.prio = prio;
	prm.p_const_syslog = p_syslog;
	KernelLibrarySyslog(&prm);
	return(prm.return_er);
}

StatusType
syslog_msk_log(uint32 lowmask)
{
	KERLIBSLG_PRM prm;

	prm.index = KERLIBSLG_ID_SYSLOG_MSK_LOG;
	prm.lowmask = lowmask;
	KernelLibrarySyslog(&prm);
	return(prm.return_er);
}
