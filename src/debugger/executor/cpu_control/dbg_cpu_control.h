#ifndef _DBG_CPU_CONTROL_H_
#define _DBG_CPU_CONTROL_H_

#include "std_types.h"
#include "object_container.h"

/*
 * 共通機能
 */
extern void cpuctrl_init(void);
extern bool cpuctrl_is_debug_mode(void);
extern void cpuctrl_set_debug_mode(bool on);

extern bool cpuctrl_is_force_debug_mode(void);
extern void cpuctrl_set_force_debug_mode(void);


extern void cpuctrl_set_force_break(void);
extern void cpuctrl_set_current_debugged_core(CoreIdType core_id);
extern void cpuctrl_clr_current_debugged_core(void);
extern bool cpuctrl_get_current_debugged_core(CoreIdType *core_id);
extern void dbg_cpu_control_print_source(uint32 pc);
extern char *dbg_cpu_control_get_print_args(void);
extern void dbg_cpu_control_update_editor(void);
extern void dbg_cpu_debug_mode_set(uint32 core_id, bool dbg_mode);
extern bool dbg_cpu_debug_mode_get(uint32 core_id);
extern void dbg_cpu_callback_start(uint32 pc, uint32 sp);
extern void dbg_cpu_callback_start_nodbg(uint32 pc, uint32 sp);

/*
 * break機能
 */
#define DBG_CPU_CONTROL_BREAK_SETSIZE	128

typedef enum {
	BREAK_POINT_TYPE_FOREVER,
	BREAK_POINT_TYPE_ONLY_ONCE,
} BreakPointEumType;
extern bool cpuctrl_is_break_point(uint32 addr);
extern bool cpuctrl_get_break(uint32 index, uint32 *addrp);
extern bool cpuctrl_set_break(uint32 addr, BreakPointEumType type);
extern bool cpuctrl_del_break(uint32 index);
extern void cpuctrl_del_all_break(BreakPointEumType type);

/*
 * データウォッチ機能
 */
#define DBG_CPU_CONTROL_WATCH_DATA_SETSIZE	128

typedef enum {
	DATA_WATCH_POINT_TYPE_READ,
	DATA_WATCH_POINT_TYPE_WRITE,
	DATA_WATCH_POINT_TYPE_RW,
} DataWatchPointEumType;
extern int cpuctrl_is_break_read_access(uint32 access_addr, uint32 size);
extern int cpuctrl_is_break_write_access(uint32 access_addr, uint32 size);
extern bool cpuctrl_get_data_watch_point(uint32 index, uint32 *addrp, uint32 *sizep, DataWatchPointEumType *type);
extern bool cpuctrl_set_data_watch(DataWatchPointEumType watch_type, uint32 addr, uint32 size);
extern bool cpuctrl_del_data_watch_point(uint32 delno);
extern void cpuctrl_del_all_data_watch_points(void);

#define ACCESS_TYPE_READ	0x01
#define ACCESS_TYPE_WRITE	0x02
/*
 * データアクセス情報取得機能
 */
typedef struct {
	uint8		access_type;
	CoreIdType 	core_id;
	uint32 		sp;
	uint32		funcid;
	uint64 		access_num;
	uint64		access_time;
} DataAccessContextType;
typedef struct {
	uint64					access_num;
	ObjectContainerType		*access_context;
} DataAccessInfoHeadType;
typedef struct {
	uint32						region_type;
	DataAccessInfoHeadType		head;
} DataAccessInfoType;

extern DataAccessInfoType *cpuctrl_get_func_access_info_table(const char* glname);
extern DataAccessInfoType *cpuctrl_get_func_access_info_table_glid(uint32 glid);

/*
 * cont機能
 */
extern void cpuctrl_set_cont_clocks(bool is_timeout, uint64 cont_clocks);
extern bool cpuctrl_is_timeout_cont_clocks(CoreIdType core_id);

/*
 * 関数ログ機能
 */
#define DBG_FUNCLOG_TRACE_SIZE			1024
extern void cpuctrl_set_func_log_trace(uint32 coreId, uint32 pc, uint32 sp);
extern char *cpuctrl_get_func_log_trace_info(uint32 coreId, uint32 bt_number, uint32 *funcpcoff, uint32 *funcid, uint32 *sp);
extern uint32 cpuctrl_get_func_log_trace_num(uint32 coreId);

/*
 * profile機能
 */
typedef struct {
	/*
	 * 呼び出し回数
	 */
	uint64 call_num;
	/*
	 * 関数呼び出し開始時の時間
	 */
	uint64 start_time;
	/*
	 * 最後にプログラムカウンタが関数内に滞留していた時間
	 */
	uint64 func_time;
	/*
	 * 関数が他関数を呼び出していた総合時間
	 * 再起呼び出し時間は含める
	 */
	uint64 total_time;
	/*
	 * 再起呼び出し回数(一時管理用)
	 */
	uint64 recursive_num;
	/*
	 * 関数入場時のスタックポインタアドレス
	 */
	uint32 sp_func_enter;
} CpuProfileType;
extern void cpuctrl_profile_collect(uint32 coreId, uint32 pc);
extern void cpuctrl_profile_get(uint32 coreId, uint32 funcid, CpuProfileType *profile);

/*
 * 関数フレーム記録
 */
#define DBG_STACK_LOG_SIZE				1024
#define DBG_STACK_NUM					1024
extern void cpuctrl_set_stack_pointer(uint32 sp);
extern Std_ReturnType cpuctrl_get_stack_pointer(int glid, uint32 bt_number, uint32 *sp);

#endif /* _DBG_CPU_CONTROL_H_ */
