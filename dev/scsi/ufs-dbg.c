#include <reg.h>
#include <dev/ufs.h>

static void print_scsi_cmd(scm * pscm)
{
	int i, len;
	if (!pscm)
		return;

	dprintf(INFO, "LUN %d\n\tCMD(%u):", pscm->sdev->lun, pscm->cdb[0]);
	for (i = 0; i < MAX_CDB_SIZE; i++) {
		dprintf(INFO, " %02x", pscm->cdb[i]);
	}
	dprintf(INFO, "\n");
	if (pscm->datalen) {
		len = pscm->datalen;
		if (len > 16)
			len = 16;
		dprintf(INFO, "\tData(%d):", pscm->datalen);
		for (i = 0; i < len; i++) {
			dprintf(INFO, " %02x", pscm->buf[i]);
		}
		dprintf(INFO, "\n");
	}
}

static void print_ufs_hci_sfr(struct ufs_host *ufs, int level)
{
	int i, len;

	printf("UFS registers\n");
	if (level > 1)
		len = 0x200;
	else
		len = 0x100;
	for (i = 0; i < len / 4; i++) {
		if ((i & 3) == 0)
			printf("%08llx :", ((u64) (ufs->ioaddr) + 4 * i));
		printf(" %08x", *((u32 *) (ufs->ioaddr) + i));
		if ((i & 3) == 3)
			printf("\n");
	}
}

static void print_ufs_hci_utrd(struct ufs_host *ufs, int level)
{
	int i;

	printf("UTP Transfer Request Descriptor (0x%08lx)\n", (ulong)ufs->utrd_addr);
	for (i = 0; i < 8; i++) {
		printf(" %08x", *((u32 *) (ufs->utrd_addr) + i));
		if ((i & 3) == 3)
			printf("\n");
	}
}

static void print_ufs_hci_cmd(struct ufs_host *ufs, int level)
{
	int i, len;

	len = level * 4;
	printf("UTP Command Descriptor\nCommand UPIU (0x%08lx)\n",
			(ulong)&(ufs->cmd_desc_addr->command_upiu));
	for (i = 0; i < len; i++) {
		printf(" %08x", *((u32 *) (&ufs->cmd_desc_addr->command_upiu) + i));
		if ((i & 3) == 3)
			printf("\n");
	}
}

static void print_ufs_hci_resp(struct ufs_host *ufs, int level)
{
	int i, len;

	len = level * 4;
	printf("Response UPIU (0x%08lx)\n", (ulong)&(ufs->cmd_desc_addr->response_upiu));
	for (i = 0; i < len; i++) {
		printf(" %08x", *((u32 *) (&ufs->cmd_desc_addr->response_upiu) + i));
		if ((i & 3) == 3)
			printf("\n");
	}
}

static void print_ufs_hci_prdt(struct ufs_host *ufs, int level)
{
	int i, len;

	len = ufs->utrd_addr->prdt_len / 4;
	if (len > 4 * level)
		len = 4 * level;
	printf("PRDT (0x%08lx) length %d\n", (ulong)ufs->cmd_desc_addr->prd_table,
			ufs->utrd_addr->prdt_len);
	for (i = 0; i < len; i++) {
		printf(" %08x", *((u32 *) (ufs->cmd_desc_addr->prd_table) + i));
		if ((i & 3) == 3)
			printf("\n");
	}
}

static void print_ufs_hci_data(struct ufs_host *ufs, int level)
{
	u64 prdt_addr;
	int i, len;

	prdt_addr = ((u64) ufs->cmd_desc_addr->prd_table[0].base_addr) |
		((u64) ufs->cmd_desc_addr->prd_table[0].upper_addr << UFS_BIT_LEN_OF_DWORD);

	len = ufs->cmd_desc_addr->prd_table[0].size + 1;
	printf("Data (addr=0x%016llx, total length=%d, first prd length=%d)\n", prdt_addr,
			(u32)(ufs->cmd_desc_addr->command_upiu.tsf[0]), len);
	if (len > 16 * level)
		len = 16 * level;
	for (i = 0; i < len; i++) {
		printf(" %02x",
				*((u8 *) (prdt_addr + i)));
		if ((i & 15) == 15)
			printf("\n");
	}
}

static void print_ufs_config_desc_units(struct ufs_config_desc *desc, u32 alloc_unit)
{
	int i;
	u32 value;

	printf("----------------------------------------------------------------------\n");
	printf("\t\t\tLUN0\tLUN1\tLUN2\tLUN3\tLUN4\tLUN5\tLUN6\tLUN7\n");
	printf("LU en");
	for (i = 0; i < 8; i++) {
		printf("\t\t");
		if (desc->unit[i].bLUEnable == 1)
			printf("en");
		else
			printf("dis");
	}
	printf("\n");

	printf("Boot\t");
	for (i = 0; i < 8; i++) {
		printf("\t");
		switch (desc->unit[i].bBootLunID) {
		case 1:
			printf("BootA");
			break;
		case 2:
			printf("BootB");
			break;
		default:
			printf("\t");
			break;
		}
	}
	printf("\n");

	printf("WP\t");
	for (i = 0; i < 8; i++) {
		printf("\t\t");
		switch (desc->unit[i].bLUWriteProtect) {
		case 1:
			printf("WP");
			break;
		case 2:
			printf("perWP");
			break;
		}
	}
	printf("\n");

	printf("Type\t");
	for (i = 0; i < 8; i++) {
		printf("\t");
		switch (desc->unit[i].bMemoryType) {
		case 0:
			printf("Normal");
			break;
		case 1:
			printf("SysCode");
			break;
		case 2:
			printf("NonPer");
			break;
		default:
			printf("Type%d", desc->unit[i].bMemoryType - 2);
			break;
		}
	}
	printf("\n");

	printf("Capa");
	for (i = 0; i < 8; i++) {
		printf("\t\t");
		value = be32_to_cpu(desc->unit[i].dNumAllocUnits) * alloc_unit;
		if (value < 1024)
			printf("%dK", value);
		else if (value < 1024 * 1024)
			printf("%dM", value / 1024);
		else
			printf("%dG", value / (1024 * 1024));
	}
	printf("\n");

	printf("BlSize");
	for (i = 0; i < 8; i++) {
		printf("\t\t");
		value = 1 << desc->unit[i].bLogicalBlockSize;
		if (value < 1024)
			printf("%dK", value);
		else if (value < 1024 * 1024)
			printf("%dM", value / 1024);
		else
			printf("%dG", value / (1024 * 1024));
	}
	printf("\n");
}

static void print_ufs_config_desc_summary(struct ufs_config_desc *desc, u32 alloc_unit)
{
	printf("bConfDescContinue \t\t\t%d ea\n", desc->header.bConfDescContinue);
	if (desc->header.bBootEnable == 1)
		printf("Boot feature\t\t\tenabled\n");
	else
		printf("Boot feature\t\t\tdisabled\n");
	if (desc->header.bDescrAccessEn == 1)
		printf("Descriptor access\t\tenabled\n");
	else
		printf("Descriptor access\t\tdisabled\n");
	if (desc->header.bInitPowerMode == 1)
		printf("Initial Power Mode\t\tActive Mode\n");
	else
		printf("Initial Power Mode\t\tUFS-Sleep Mode\n");
	if (desc->header.bHighPriorityLUN == 0x7f)
		printf("All logical unit have the same priority\n");
	else
		printf("High priority logical unit\t%d\n",
		       desc->header.bHighPriorityLUN);

	print_ufs_config_desc_units(desc, alloc_unit);
}

#if 0
static void print_ufs_information(struct ufs_host *ufs)
{
	int i;
	u32 capacity, alloc_unit, value;
	struct ufs_host *ufs = get_cur_ufs_host();
	if (!ufs)
		return;

	dprintf(INFO, "----------------------------------------------------------------------\n");
	dprintf(INFO, "UFS device information\n");
	dprintf(INFO, "----------------------------------------------------------------------\n");
	capacity = (2048 * 1024) * be32_to_cpu(ufs->geometry_desc.qTotalRawDeviceCapacity_h)
	    + be32_to_cpu(ufs->geometry_desc.qTotalRawDeviceCapacity_l) / (2 * 1024);
	dprintf(INFO, "Capacity\t\t\t%d Gbytes (%dMbytes)\n", capacity / 1024, capacity);
	dprintf(INFO, "Erase block size\t\t%d Kbytes\n", be32_to_cpu(ufs->geometry_desc.dSegmentSize) / 2);
	alloc_unit =
	    ufs->geometry_desc.bAllocationUnitSize * be32_to_cpu(ufs->geometry_desc.dSegmentSize) /
	    2;
	dprintf(INFO, "Allocation unit size\t\t%d Kbytes\n", alloc_unit);
	dprintf(INFO, "Address block size\t\t%d Kbytes\n", ufs->geometry_desc.bMinAddrBlockSize / 2);
	dprintf(INFO, "Optimal read block size\t\t%d Kbytes\n",
	       ufs->geometry_desc.bOptimalReadBlockSize / 2);
	dprintf(INFO, "Optimal write block size\t%d Kbytes\n",
	       ufs->geometry_desc.bOptimalReadBlockSize / 2);
	dprintf(INFO, "Supported memory type\n");
	value = be16_to_cpu(ufs->geometry_desc.wSupportedMemoryTypes);
	if (value & (1 << 0))
		dprintf(INFO, "\tNormal memory\n");
	if (value & (1 << 1))
		dprintf(INFO, "\tSystem code memory\n");
	if (value & (1 << 2))
		dprintf(INFO, "\tNon-Persistent memory\n");
	if (value & (1 << 3))
		dprintf(INFO, "\tEnhanced memory memory type 1\n");
	if (value & (1 << 4))
		dprintf(INFO, "\tEnhanced memory memory type 2\n");
	if (value & (1 << 5))
		dprintf(INFO, "\tEnhanced memory memory type 3\n");
	if (value & (1 << 6))
		dprintf(INFO, "\tEnhanced memory memory type 4\n");
	if (value & (1 << 15))
		dprintf("\tRPMB memory\n");

	print_ufs_config_desc_summary(&ufs->config_desc, alloc_unit);

	printf("----------------------------------------------------------------------\n");
}
#endif

void print_ufs_upiu(struct ufs_host *ufs, int print_level)
{
	u32 level;

	if (!ufs)
		return;

	dprintf(INFO, "\n");

	level = print_level & 0x7;
	if (level)
		print_ufs_hci_sfr(ufs, level);

	level = (print_level >> 4) & 0x7;
	if (level)
		print_ufs_hci_utrd(ufs, level);

	level = (print_level >> 8) & 0x7;
	if (level++)
		print_ufs_hci_cmd(ufs, level);

	level = (print_level >> 12) & 0x7;
	if (level++)
		print_ufs_hci_resp(ufs, level);

	if (ufs->utrd_addr->prdt_len) {
		level = (print_level >> 16) & 0x7;
		if (level)
			print_ufs_hci_prdt(ufs, level);

		level = (print_level >> 20) & 0x7;
		if (level++)
			print_ufs_hci_data(ufs, level);
	}

	level = (print_level >> 24) & 0x7;
	if (level) {
		print_scsi_cmd(ufs->scsi_cmd);
	}
	dprintf(INFO, "\n");
}

void print_ufs_desc(u8 * desc)
{
	int i, len, type;
	if (!desc)
		return;

	len = desc[0];
	type = desc[1];

	dprintf(INFO, "Descriptor length 0x%02x, type 0x%02x\n", len, type);

	for (i = 0; i < len; i++) {
		if ((i & 0xf) == 0)
			dprintf(INFO, "%02x :", i);
		dprintf(INFO, " %02x", desc[i]);
		if ((i & 0xf) == 0xf)
			dprintf(INFO, "\n");
	}
}

void print_ufs_device_desc(u8 * desc)
{
	struct ufs_device_desc *dp = (struct ufs_device_desc *)desc;
	if (!desc)
		return;

	printf("Device Descriptor\n");
	printf("------------------------------------------------------\n");
	printf("bLength = 0x%02x (1Fh)\n", dp->bLength);
	printf("bDescriptorType = 0x%02x (00h)\n", dp->bDescriptorType);
	printf("bDevice = 0x%02x (00h device, others reserved)\n", dp->bDevice);
	printf("bDeviceClass = 0x%02x (00h Mass storage)\n", dp->bDeviceClass);
	printf
	    ("bDeviceSubClass = 0x%02x (00h embedded bootable, 01h embedded non-bootable, 02h removable non-bootable)\n",
	     dp->bDeviceSubClass);
	printf("bProtocol = 0x%02x (00h scsi)\n", dp->bProtocol);
	printf("bNumberLU = 0x%02x (01h~08h, default 01h)\n", dp->bNumberLU);
	printf("iNumberWLU = 0x%02x (04h)\n", dp->iNumberWLU);
	printf("bBootEnable = 0x%02x (00h boot disabled, 01h boot enabled)\n", dp->bBootEnable);
	printf("bDescrAccessEn = 0x%02x (00h device desc access disabled, 01h access enabled)\n",
	       dp->bDescrAccessEn);

	printf("bInitPowerMode = 0x%02x (00h UFS sleep mode, 01h active mode)\n",
	       dp->bInitPowerMode);
	printf("bHighPriorityLUN = 0x%02x (00h~07h, 7Fh same priority)\n", dp->bHighPriorityLUN);
	printf("bSecureRemovalType = 0x%02x (00h~03h)\n", dp->bSecureRemovalType);
	printf("bSecurityLU = 0x%02x (00h not support, 01h RPMB)\n", dp->bSecurityLU);
	printf("bInitActiveICCLevel = 0x%02x (00h~0Fh, default 00h)\n", dp->bInitActiveICCLevel);
	printf("wSpecVersion = 0x%04x\n", be16_to_cpu(dp->wSpecVersion));
	printf("wManufactureData = 0x%04x (MM/YY)\n", be16_to_cpu(dp->wManufactureData));
	printf("iManufacturerName = 0x%02x\n", dp->iManufacturerName);
	printf("iProductName = 0x%02x\n", dp->iProductName);
	printf("iSerialNumber = 0x%02x\n", dp->iSerialNumber);

	printf("iOemID = 0x%02x\n", dp->iOemID);
	printf("wManufacturerID = 0x%04x\n", be16_to_cpu(dp->wManufacturerID));
	printf("bUD0BaseOffset = 0x%02x (10h)\n", dp->bUD0BaseOffset);
	printf("bUDConfigPlength = 0x%02x (10h)\n", dp->bUDConfigPlength);
	printf("bDeviceRTTCap = 0x%02x (minimum value 02h)\n", dp->bDeviceRTTCap);
	printf("wPeriodicRTCUpdate = 0x%04x\n", be16_to_cpu(dp->wPeriodicRTCUpdate));
	printf("------------------------------------------------------\n");
}

void print_ufs_configuration_desc(u8 * desc)
{
	int i;
	struct ufs_config_desc *dp = (struct ufs_config_desc *)desc;
	if (!desc)
		return;

	printf("Configuration Descriptor\n");
	printf("------------------------------------------------------\n");
	printf("Configuration Descriptor Header\n");
	printf("bLength = 0x%02x (90h)\n", dp->header.bLength);
	printf("bDescriptorType = 0x%02x (01h)\n", dp->header.bDescriptorType);
	printf("bConfDescContinue = 0x%02x\n", dp->header.bConfDescContinue);
	printf("bBootEnable = 0x%02x\n", dp->header.bBootEnable);
	printf("bDescrAccessEn = 0x%02x\n", dp->header.bDescrAccessEn);
	printf("bInitPowerMode = 0x%02x\n", dp->header.bInitPowerMode);
	printf("bHighPriorityLUN = 0x%02x\n", dp->header.bHighPriorityLUN);
	printf("bSecureRemovalType = 0x%02x\n", dp->header.bSecureRemovalType);
	printf("bInitActiveICCLevel = 0x%02x\n", dp->header.bInitActiveICCLevel);
	printf("wPeriodicRTCUpdate = 0x%04x\n", be16_to_cpu(dp->header.wPeriodicRTCUpdate));

	for (i = 0; i < 8; i++) {
		printf("Unit Descriptor configurable parameters (%02xh)\n", i);
		printf("\tbLUEnable = 0x%02x\n", dp->unit[i].bLUEnable);
		printf("\tbBootLunID = 0x%02x\n", dp->unit[i].bBootLunID);
		printf("\tbLUWriteProtect = 0x%02x\n", dp->unit[i].bLUWriteProtect);
		printf("\tbMemoryType = 0x%02x\n", dp->unit[i].bMemoryType);
		printf("\tdNumAllocUnits = 0x%08x\n", be32_to_cpu(dp->unit[i].dNumAllocUnits));
		printf("\tbDataReliability = 0x%02x\n", dp->unit[i].bDataReliability);
		printf("\tbLogicalBlockSize = 0x%02x\n", dp->unit[i].bLogicalBlockSize);
		printf("\tbProvisioningType = 0x%02x\n", dp->unit[i].bProvisioningType);
		printf("\twContextCapabilities = 0x%04x\n",
		       be16_to_cpu(dp->unit[i].wContextCapabilities));
	}
	dprintf(INFO, "------------------------------------------------------\n");
}

void print_ufs_geometry_desc(u8 * desc)
{
	struct ufs_geometry_desc *dp = (struct ufs_geometry_desc *)desc;
	if (!desc)
		return;

	printf("Geometry Descriptor\n");
	printf("------------------------------------------------------\n");
	printf("bLength = 0x%02x (44h)\n", dp->bLength);
	printf("bDescriptorType = 0x%02x (07h)\n", dp->bDescriptorType);
	printf("bMediaTechnology = 0x%02x (00h reserved)\n", dp->bMediaTechnology);
	printf("qTotalRawDeviceCapacity = 0x%08x%08x (512bytes)\n",
	       be32_to_cpu(dp->qTotalRawDeviceCapacity_h),
	       be32_to_cpu(dp->qTotalRawDeviceCapacity_l));
	printf("dSegmentSize = 0x%08x (512bytes)\n", be32_to_cpu(dp->dSegmentSize));
	printf("bAllocationUnitSize = 0x%02x (segments)\n", dp->bAllocationUnitSize);
	printf("bMinAddrBlockSize = 0x%02x (512bytes, minimum value 08h=4KB)\n",
	       dp->bMinAddrBlockSize);
	printf("bOptimalReadBlockSize = 0x%02x (512bytes, 0=not available)\n",
	       dp->bOptimalReadBlockSize);
	printf("bOptimalWriteBlockSize = 0x%02x (512bytes)\n", dp->bOptimalWriteBlockSize);
	printf("bMaxInBufferSize = 0x%02x (512bytes, minimum value 08h=4KB)\n",
	       dp->bMaxInBufferSize);
	printf("bMaxOutBufferSize = 0x%02x (512bytes, minimum value 08h=4KB)\n",
	       dp->bMaxOutBufferSize);
	printf("bRPMB_ReadWriteSize = 0x%02x\n", dp->bRPMB_ReadWriteSize);
	printf("bDataOrdering = 0x%02x (00h not support, 01h support)\n", dp->bDataOrdering);
	printf("bMaxContexIDNumber = 0x%02x (minimum value 05h)\n", dp->bMaxContexIDNumber);
	printf("bSysDataTagUnitSize = 0x%02x\n", dp->bSysDataTagUnitSize);
	printf("bSysDataTagResSize = 0x%02x\n", dp->bSysDataTagResSize);
	printf("bSupportedSecRTypes = 0x%02x\n", dp->bSupportedSecRTypes);
	printf("wSupportedMemoryTypes = 0x%04x\n", be16_to_cpu(dp->wSupportedMemoryTypes));
	printf("dSystemCodeMaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dSystemCodeMaxNAllocU));
	printf("wSystemCodeCapAdjFac = 0x%04x\n", be16_to_cpu(dp->wSystemCodeCapAdjFac));
	printf("dNonPersistMaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dNonPersistMaxNAllocU));
	printf("wNonPersistCapAdjFac = 0x%04x\n", be16_to_cpu(dp->wNonPersistCapAdjFac));
	printf("dEnhanced1MaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dEnhanced1MaxNAllocU));
	printf("wEnhanced1CapAdjFac = 0x%0x\n", be16_to_cpu(dp->wEnhanced1CapAdjFac));
	printf("dEnhanced2MaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dEnhanced2MaxNAllocU));
	printf("wEnhanced2CapAdjFac = 0x%04x\n", be16_to_cpu(dp->wEnhanced2CapAdjFac));
	printf("dEnhanced3MaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dEnhanced3MaxNAllocU));
	printf("wEnhanced3CapAdjFac = 0x%04x\n", be16_to_cpu(dp->wEnhanced3CapAdjFac));
	printf("dEnhanced4MaxNAllocU = 0x%08x\n", be32_to_cpu(dp->dEnhanced4MaxNAllocU));
	printf("wEnhanced4CapAdjFac = 0x%04x\n", be16_to_cpu(dp->wEnhanced4CapAdjFac));
	printf("------------------------------------------------------\n");
}

void print_ufs_flags(union ufs_flags *flags)
{
	dprintf(INFO, "----------------------------------------------------------------------\n");
	dprintf(INFO, "UFS device flags\n");
	dprintf(INFO, "----------------------------------------------------------------------\n");
	dprintf(INFO, "UFS flag : fDeviceInit: %d\n", flags->flag.fDeviceInit);
	dprintf(INFO, "UFS flag : fPermanentWPEn: %d\n", flags->flag.fPermanentWPEn);
	dprintf(INFO, "UFS flag : fPowerOnWPEn: %d\n", flags->flag.fPowerOnWPEn);
	dprintf(INFO, "UFS flag : fBackgroundOpsEn: %d\n", flags->flag.fBackgroundOpsEn);
}

void exynos_ufs_get_sfr(struct ufs_host *ufs,
					struct exynos_ufs_sfr_log *cfg)
{
	u32 sel_api = 0;

	printf("UFS Get SFR Dump\n");

	while (cfg) {
		if (!cfg->name)
			break;

		if (cfg->offset >= LOG_STD_HCI_SFR) {
			/* Select an API to get SFRs */
			sel_api = cfg->offset;
		} else {
			/* Fetch value */
			if (sel_api == LOG_STD_HCI_SFR)
				cfg->val = readl(ufs->ioaddr + cfg->offset);
			else if (sel_api == LOG_VS_HCI_SFR)
				cfg->val = readl(ufs->vs_addr + cfg->offset);
			else if (sel_api == LOG_FMP_SFR)
				cfg->val = readl(ufs->fmp_addr + cfg->offset);
			else if (sel_api == LOG_UNIPRO_SFR)
				cfg->val = readl(ufs->unipro_addr + cfg->offset);
			else if (sel_api == LOG_PMA_SFR)
				cfg->val = readl(ufs->phy_pma + cfg->offset);
			else
				cfg->val = 0xFFFFFFFF;
		}
		printf("%s(0x%04x):\t\t\t\t0x%08x\n",
				cfg->name, cfg->offset, cfg->val);

		/* Next SFR */
		cfg++;
	}
}

void exynos_ufs_get_attr(struct ufs_host *ufs,
					struct exynos_ufs_attr_log *cfg)
{
	int err = 0;

	printf("UFS Get ATTR Dump\n");

	while (cfg) {
		if (cfg->offset == 0)
			break;

		ufs->uic_cmd->uiccmdr = UIC_CMD_DME_GET;
		ufs->uic_cmd->uiccmdarg1 = cfg->offset;

		writel(ufs->uic_cmd->uiccmdarg1, (ufs->ioaddr + REG_UIC_COMMAND_ARG_1));
		writel(ufs->uic_cmd->uiccmdr, (ufs->ioaddr + REG_UIC_COMMAND));

		ufs->timeout = ufs->uic_cmd_timeout;	/* usec unit */
		while (UFS_IN_PROGRESS == (err = handle_ufs_int(ufs, 1)));
		writel(readl(ufs->ioaddr + REG_INTERRUPT_STATUS),
				ufs->ioaddr + REG_INTERRUPT_STATUS);

		cfg->res = readl(ufs->ioaddr + REG_UIC_COMMAND_ARG_2);
		cfg->val = readl(ufs->ioaddr + REG_UIC_COMMAND_ARG_3);

		printf("0x%04x:\t\t0x%08x\t\t0x%08x\n",
				cfg->offset, cfg->res, cfg->val);
out:
		if (err)
			printf("Failed to fetch a value of %x err: %d \n",
					ufs->uic_cmd->uiccmdarg1, err);

		/* Next attribute */
		cfg++;
	}
}

void exynos_ufs_get_uic_info(struct ufs_host *ufs)
{
	exynos_ufs_get_sfr(ufs, ufs->debug.sfr);
	exynos_ufs_get_attr(ufs, ufs->debug.attr);
}
