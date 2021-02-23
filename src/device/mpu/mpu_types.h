#ifndef _MPU_TYPES_H_
#define _MPU_TYPES_H_

#include "std_types.h"

typedef enum {
	READONLY_MEMORY = 0,
	GLOBAL_MEMORY,
	LOCAL_MEMORY,
	PRIVATE_MEMORY,
	DEVICE,
	REGION_UNKNOWN,
} MpuAddressRegionEnumType;

extern MpuAddressRegionEnumType mpu_address_region_type_get(uint32 addr, std_bool *is_malloc);

#define MPU_ADDRESS_REGION_MASK_ALL				0xFFFFFFFF
#define MPU_ADDRESS_REGION_PERM_ALL				0xFFFFFFFF

struct mpu_address_region_operation_type;
typedef struct {
	MpuAddressRegionEnumType					type;
	bool										is_malloc;
	/*
	 * 本メモリ領域にアクセスできるcore_idのビットマップ
	 * ビット位置がCPUのコアIDに対応する
	 * 0：許可されてない
	 * 1:許可されている
	 */
	uint32										permission;
	uint32										start;
	uint32										size;
	uint32										mask;
	uint8										*data;
	struct mpu_address_region_operation_type	*ops;
} MpuAddressRegionType;

typedef struct mpu_address_region_operation_type {
	Std_ReturnType (*get_data8) (MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint8 *data);
	Std_ReturnType (*get_data16) (MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint16 *data);
	Std_ReturnType (*get_data32) (MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint32 *data);

	Std_ReturnType (*put_data8) (MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint8 data);
	Std_ReturnType (*put_data16) (MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint16 data);
	Std_ReturnType (*put_data32) (MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint32 data);

	Std_ReturnType (*get_pointer) (MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint8 **data);
} MpuAddressRegionOperationType;

extern MpuAddressRegionOperationType	default_memory_operation;


#ifdef CPU_CONFIG_MEMORY_PROTECTION_ENABLE
#include "target_cpu.h"
extern bool cpu_has_permission(CoreIdType core_id, MpuAddressRegionEnumType region_type, CpuMemoryAccessType access_type, uint32 addr, uint32 size);
#define CPU_HAS_PERMISSION(core_id, region_type, access_type, addr, size) cpu_has_permission(core_id, region_type, access_type, addr, size)
#else
#define CPU_HAS_PERMISSION(core_id, region_type, access_type, addr, size) TRUE
#endif /* MPU_CONFIG_PROTECTION_ENABLE */

#endif /* _MPU_TYPES_H_ */
