#ifndef __NVBIOS_PMU_H__
#define __NVBIOS_PMU_H__

struct nvbios_pmuT {
};

u32 nvbios_pmuTe(struct nouveau_bios *, u8 *ver, u8 *hdr, u8 *cnt, u8 *len);
u32 nvbios_pmuTp(struct nouveau_bios *, u8 *ver, u8 *hdr, u8 *cnt, u8 *len,
		 struct nvbios_pmuT *);

struct nvbios_pmuE {
	u8  type;
	u32 data;
};

u32 nvbios_pmuEe(struct nouveau_bios *, int idx, u8 *ver, u8 *hdr);
u32 nvbios_pmuEp(struct nouveau_bios *, int idx, u8 *ver, u8 *hdr,
		 struct nvbios_pmuE *);

struct nvbios_pmuR {
	u32 boot_addr_pmu;
	u32 boot_addr;
	u32 boot_size;
	u32 code_addr_pmu;
	u32 code_addr;
	u32 code_size;
	u32 init_addr_pmu;

	u32 data_addr_pmu;
	u32 data_addr;
	u32 data_size;
	u32 args_addr_pmu;
};

bool nvbios_pmuRm(struct nouveau_bios *, u8 type, struct nvbios_pmuR *);

#endif
