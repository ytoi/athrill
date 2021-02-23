#include "loader/loader.h"
#include "cpu.h"
#include "mpu.h"
#include "mpu_ops.h"
#include "loader/elf.h"
#include "elf_section.h"
#include "elf_dwarf_line.h"
#include "elf_dwarf_abbrev.h"
#include "elf_dwarf_info.h"
#include "elf_dwarf_loc.h"
#include "elf_dwarf_data_type.h"
#include "athrill_device.h"
#include "assert.h"
#include <string.h>
#include <stdio.h>

static Std_ReturnType Elf_Check(const Elf32_Ehdr *elf_image);
static Std_ReturnType Elf_LoadProgram(const Elf32_Ehdr *elf_image, MemoryAddressMapType *memap);

Std_ReturnType binary_load(uint8 *binary_data, uint32 load_addr, uint32 binary_data_len)
{
	Std_ReturnType err;
	uint8 *data = NULL;

	err = mpu_get_pointer(CPU_CONFIG_CORE_ID_0, load_addr, &data);
	if (err != STD_E_OK) {
		return err;
	}

	memcpy(data, binary_data, binary_data_len);

	return STD_E_OK;
}

Std_ReturnType elf_load(uint8 *elf_data, MemoryAddressMapType *memap)
{
	Std_ReturnType err;
	uint32 disable_debug_data_type = 0;

	err = Elf_Check((const Elf32_Ehdr*)elf_data);
	if (err != STD_E_OK) {
		return err;
	}
	err = Elf_LoadProgram((const Elf32_Ehdr*)elf_data, memap);
	if (err != STD_E_OK) {
		return err;
	}
	err = elf_symbol_load(elf_data);
	if (err != STD_E_OK) {
		return err;
	}
	err = elf_dwarf_line_load(elf_data);
	if (err != STD_E_OK) {
		return err;
	}
	err = elf_dwarf_abbrev_load(elf_data);
	if (err != STD_E_OK) {
		return err;
	}
	err = elf_dwarf_info_load(elf_data);
	if (err != STD_E_OK) {
		return err;
	}
	err = elf_dwarf_loc_load(elf_data);
	if (err != STD_E_OK) {
		return err;
	}
	cpuemu_get_devcfg_value("DEBUG_FUNC_DISABLE_DEBUG_DATA_TYPE", &disable_debug_data_type);
	if (disable_debug_data_type == 0) {
		dwarf_build_data_type_set();
	}

	return err;
}


static Std_ReturnType Elf_Check(const Elf32_Ehdr *elf_image)
{
	// magic check
	if (IS_ELF(*elf_image) == FALSE) {
		return STD_E_INVALID;
	}
	if (elf_image->e_ident[EI_CLASS] != ELFCLASS32) {
		return STD_E_INVALID;
	}
	if (elf_image->e_ident[EI_DATA] != ELFDATA2LSB) {
		return STD_E_INVALID;
	}
	if (elf_image->e_ident[EI_VERSION] != EV_CURRENT) {
		return STD_E_INVALID;
	}
	if (elf_image->e_ident[EI_OSABI] != ELFOSABI_SYSV) {
		return STD_E_INVALID;
	}
	if (elf_image->e_ident[EI_ABIVERSION] != 0U) {
		return STD_E_INVALID;
	}
	if (elf_image->e_type != ET_EXEC) {
		return STD_E_INVALID;
	}
	if (elf_image->e_machine != ELF_MACHINE_TYPE) {
		return STD_E_INVALID;
	}
	if (elf_image->e_version != EV_CURRENT) {
		return STD_E_INVALID;
	}
	return STD_E_OK;
}

static void set_malloc_region(MemoryAddressMapType *memap, uint32 i)
{
	int j;
	uint32 start = memap->ram[i].start;
	uint32 unit_num = memap->ram[i].size / MPU_MALLOC_REGION_UNIT_SIZE;

	for (j = 0; j < unit_num; j++) {
		mpu_address_set_malloc_region(start, MPU_MALLOC_REGION_UNIT_SIZE * 1024);
		start += (MPU_MALLOC_REGION_UNIT_SIZE * 1024);
	}
	return;
}
static void set_cache_code_from_map(const MemoryAddressType *memory_map)
{
	if (memory_map->region_executable != FALSE) {
		virtual_cpu_cache_code_add_with_check((memory_map->size * 1024), memory_map->start);
		printf("MEMSETTING SET CACHE RIGION:addr=0x%x size=%u [KB]\n", memory_map->start, memory_map->size);
	}
	return;
}
static void set_cache_code_from_elf(const Elf32_Ehdr *elf_image)
{
	uint32 i;
	Elf32_Phdr *phdr;
	for (i = 0; i < elf_image->e_phnum; i++) {
		phdr = (Elf32_Phdr*) (
				((uint8_t*)elf_image)
				+ elf_image->e_phoff
				+ (elf_image->e_phentsize * i)
				);
		//printf("p_type=0x%x\n", phdr->p_type);
		//printf("p_paddr=0x%x\n", phdr->p_paddr);
		//printf("p_flags=0x%x\n", phdr->p_flags);
		if ((phdr->p_flags & (PF_X)) == PF_X) {
			virtual_cpu_cache_code_add_with_check(phdr->p_memsz, phdr->p_vaddr);
			printf("ELF SET CACHE RIGION:addr=0x%x size=%u [KB]\n", phdr->p_vaddr, phdr->p_memsz / 1024);
		}
	}
	return;
}

static Std_ReturnType Elf_LoadProgram(const Elf32_Ehdr *elf_image, MemoryAddressMapType *memap)
{
	Std_ReturnType err;
	uint32_t i;
	Elf32_Phdr *phdr;
	uint8 *ptr = NULL;

	/*
	 * alloc memory region
	 */
	for (i = 0; i < memap->rom_num; i++) {
		ptr = mpu_address_set_rom_ram((MpuAddressGetType)memap->rom[i].type, memap->rom[i].start, memap->rom[i].size * 1024, NULL);
		if (ptr == NULL) {
			printf("Invalid elf file: can not load rom addr=0x%x\n", memap->rom[i].start);
			return STD_E_INVALID;
		}
		set_cache_code_from_map(&memap->rom[i]);
	}
	for (i = 0; i < memap->ram_num; i++) {
		if (memap->ram[i].type != MemoryAddressImplType_MALLOC) {
			ptr = mpu_address_set_rom_ram((MpuAddressGetType)memap->ram[i].type, memap->ram[i].start, memap->ram[i].size * 1024, memap->ram[i].mmap_addr);
			if (ptr == NULL) {
				printf("Invalid elf file: can not load ram addr=0x%x\n", memap->ram[i].start);
				return STD_E_INVALID;
			}
		}
		else {
			int j;
			uint32 start = memap->ram[i].start;
			uint32 unit_num = memap->ram[i].size / MPU_MALLOC_REGION_UNIT_SIZE;

			for (j = 0; j < unit_num; j++) {
				(void)mpu_address_set_rom_ram((MpuAddressGetType)memap->ram[i].type, start, MPU_MALLOC_REGION_UNIT_SIZE * 1024, NULL);
				start += (MPU_MALLOC_REGION_UNIT_SIZE * 1024);
			}
		}
		set_cache_code_from_map(&memap->ram[i]);
	}
	for (i = 0; i < memap->ram_num; i++) {
		if (memap->ram[i].type == MemoryAddressImplType_MALLOC) {
			set_malloc_region(memap, i);
		}
	}
	for (i = 0; i < memap->dev_num; i++) {
		ptr = mpu_address_set_dev(memap->dev[i].start, memap->dev[i].size, memap->dev[i].extdev_handle);
		if (ptr == NULL) {
			printf("Invalid dev file: can not load dev addr=0x%x\n", memap->dev[i].start);
			return STD_E_INVALID;
		}
	}
	/*
	 * set cache from elf file.
	 */
	set_cache_code_from_elf(elf_image);
	/*
	 * load program from elf file.
	 */
	for (i = 0; i < elf_image->e_phnum; i++) {
		phdr = (Elf32_Phdr*) (
				((uint8_t*)elf_image)
				+ elf_image->e_phoff
				+ (elf_image->e_phentsize * i)
				);
		if (phdr->p_type != PT_LOAD) {
			continue;
		}
		/*
		 * ROM領域のみロードする．
		 */
		ptr = mpu_address_get_rom(phdr->p_paddr, phdr->p_filesz);
		if (ptr == NULL) {
			continue;
		}
		err = mpu_get_pointer(CPU_CONFIG_CORE_ID_0, phdr->p_paddr, &ptr);
		if (err != STD_E_OK) {
			printf("Invalid elf file: can not load addr=0x%x\n", phdr->p_paddr);
			return STD_E_INVALID;
		}

		memcpy(ptr,
				( ((uint8_t*)elf_image) + phdr->p_offset ),
				phdr->p_filesz);

		printf("Elf loading was succeeded:0x%x - 0x%x : %u.%u KB\n", phdr->p_paddr, phdr->p_paddr + phdr->p_memsz, phdr->p_filesz/1024, phdr->p_filesz % 1024);
	}
	return 0;
}
