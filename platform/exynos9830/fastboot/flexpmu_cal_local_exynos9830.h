struct pmucal_seq aud_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "CLK_CON_MUX_MUX_CLK_AUD_UAIF6", 0x18c00000, 0x1024, (0x3 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "AUD_CMU_AUD_CONTROLLER_OPTION", 0x18c00000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "PLL_CON0_PLL_AUD1", 0x18c00000, 0x0140, (0x1 << 4), (0x0 << 4), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "AUD_CONFIGURATION", 0x15860000, 0x1900, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "AUD_STATUS", 0x15860000, 0x1904, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq csis_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "QCH_CON_OIS_MCU_TOP_QCH_A", 0x17000000, 0x30c0, (0xffffffff << 0), (0x4 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "QCH_CON_OIS_MCU_TOP_QCH_D", 0x17000000, 0x30c4, (0xffffffff << 0), (0x4 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "QCH_CON_OIS_MCU_TOP_QCH_H", 0x17000000, 0x30cc, (0xffffffff << 0), (0x4 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "CSIS_CMU_CSIS_CONTROLLER_OPTION", 0x17000000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "CSIS_CONFIGURATION", 0x15860000, 0x1d80, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "CSIS_STATUS", 0x15860000, 0x1d84, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq dns_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "DNS_CMU_DNS_CONTROLLER_OPTION", 0x17500000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "DNS_CONFIGURATION", 0x15860000, 0x1e00, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "DNS_STATUS", 0x15860000, 0x1e04, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq dsp0_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "DSP0_CMU_DSP0_CONTROLLER_OPTION", 0x17c00000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "DSP0_CONFIGURATION", 0x15860000, 0x1f00, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "DSP0_STATUS", 0x15860000, 0x1f04, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq dsp1_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "DSP1_CMU_DSP1_CONTROLLER_OPTION", 0x17d00000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "DSP1_CONFIGURATION", 0x15860000, 0x1f80, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "DSP1_STATUS", 0x15860000, 0x1f84, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq dnc_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "DNC_CMU_DNC_CONTROLLER_OPTION", 0x19400000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "DNC_CONFIGURATION", 0x15860000, 0x2000, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "DNC_STATUS", 0x15860000, 0x2004, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq embedded_g3d_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "EMBEDDED_G3D_CONFIGURATION", 0x15860000, 0x2200, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "EMBEDDED_G3D_STATUS", 0x15860000, 0x2204, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "PLL_CON0_MUX_CLKCMU_EMBEDDED_G3D_USER", 0x18400000, 0x0600, (0x1 << 4), (0x0 << 4), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "PLL_CON0_PLL_G3D", 0x1a330000, 0x0100, (0x1 << 4), (0x0 << 4), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "PLL_CON1_PLL_G3D", 0x1a330000, 0x0104, (0xffffffff << 0), (0x10000001 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq g2d_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "G2D_CMU_G2D_CONTROLLER_OPTION", 0x18a00000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "G2D_CONFIGURATION", 0x15860000, 0x2280, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "G2D_STATUS", 0x15860000, 0x2284, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq ipp_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "IPP_CMU_IPP_CONTROLLER_OPTION", 0x17300000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "IPP_CONFIGURATION", 0x15860000, 0x2500, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "IPP_STATUS", 0x15860000, 0x2504, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq itp_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "ITP_CMU_ITP_CONTROLLER_OPTION", 0x17400000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "ITP_CONFIGURATION", 0x15860000, 0x2580, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "ITP_STATUS", 0x15860000, 0x2584, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq mfc0_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "MFC0_CMU_MFC0_CONTROLLER_OPTION", 0x18600000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "PLL_CON0_MUX_CLKCMU_MFC0_WFD_USER", 0x18600000, 0x0610, (0x1 << 4), (0x0 << 4), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "PLL_CON0_MUX_CLKCMU_MFC0_WFD_USER", 0x18600000, 0x0610, (0xffffffff << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "MFC0_CONFIGURATION", 0x15860000, 0x2600, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "MFC0_STATUS", 0x15860000, 0x2604, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq mcsc_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "MCSC_CMU_MCSC_CONTROLLER_OPTION", 0x18200000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "MCSC_CONFIGURATION", 0x15860000, 0x2880, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "MCSC_STATUS", 0x15860000, 0x2884, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq npu0_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "NPU_CMU_NPU_CONTROLLER_OPTION", 0x17E00000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "NPU0_CONFIGURATION", 0x15860000, 0x2900, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "NPU0_STATUS", 0x15860000, 0x2904, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq npu1_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "NPU_CMU_NPU_CONTROLLER_OPTION", 0x17F00000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "NPU1_CONFIGURATION", 0x15860000, 0x2980, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "NPU1_STATUS", 0x15860000, 0x2984, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq tnr_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "TNR_CMU_TNR_CONTROLLER_OPTION", 0x17700000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "TNR_CONFIGURATION", 0x15860000, 0x2c00, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "TNR_STATUS", 0x15860000, 0x2c04, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq vra_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "VRA_CMU_VRA_CONTROLLER_OPTION", 0x17600000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "VRA_CONFIGURATION", 0x15860000, 0x2c80, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "VRA_STATUS", 0x15860000, 0x2c84, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq vts_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "PLL_CON0_MUX_CLKCMU_VTS_AUD0_USER", 0x15500000, 0x0600, (0x1 << 4), (0x0 << 4), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "PLL_CON0_MUX_CLKCMU_VTS_AUD1_USER", 0x15500000, 0x0610, (0x1 << 4), (0x0 << 4), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "VTS_CMU_VTS_CONTROLLER_OPTION", 0x15500000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "VTS_CONFIGURATION", 0x15860000, 0x2d00, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "VTS_STATUS", 0x15860000, 0x2d04, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
struct pmucal_seq ssp_off[] = {
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "SSP_CMU_SSP_CONTROLLER_OPTION", 0x19600000, 0x0800, (0x1 << 24), (0x0 << 24), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WRITE, "SSP_CONFIGURATION", 0x15860000, 0x2d80, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
	PMUCAL_SEQ_DESC(PMUCAL_WAIT, "SSP_STATUS", 0x15860000, 0x2d84, (0x1 << 0), (0x0 << 0), 0, 0, 0xffffffff, 0),
};
enum pmucal_local_pdnum {
	PD_AUD,
	PD_CSIS,
	PD_DNS,
	PD_DSP0,
	PD_DSP1,
	PD_DNC,
	PD_EMBEDDED_G3D,
	PD_G2D,
	PD_IPP,
	PD_ITP,
	PD_MFC0,
	PD_MCSC,
	PD_NPU0,
	PD_NPU1,
	PD_TNR,
	PD_VRA,
	PD_VTS,
	PD_SSP,
	PMUCAL_NUM_PDS,
};

struct pmucal_pd pmucal_pd_list[PMUCAL_NUM_PDS] = {
	PMUCAL_PD_DESC(PD_AUD, "blkpwr_aud", aud_off),
	PMUCAL_PD_DESC(PD_CSIS, "blkpwr_csis", csis_off),
	PMUCAL_PD_DESC(PD_DNS, "blkpwr_dns", dns_off),
	PMUCAL_PD_DESC(PD_DSP0, "blkpwr_dsp0", dsp0_off),
	PMUCAL_PD_DESC(PD_DSP1, "blkpwr_dsp1", dsp1_off),
	PMUCAL_PD_DESC(PD_DNC, "blkpwr_dnc", dnc_off),
	PMUCAL_PD_DESC(PD_EMBEDDED_G3D, "blkpwr_embedded_g3d", embedded_g3d_off),
	PMUCAL_PD_DESC(PD_G2D, "blkpwr_g2d", g2d_off),
	PMUCAL_PD_DESC(PD_IPP, "blkpwr_ipp", ipp_off),
	PMUCAL_PD_DESC(PD_ITP, "blkpwr_itp", itp_off),
	PMUCAL_PD_DESC(PD_MFC0, "blkpwr_mfc0", mfc0_off),
	PMUCAL_PD_DESC(PD_MCSC, "blkpwr_mcsc", mcsc_off),
	PMUCAL_PD_DESC(PD_NPU0, "blkpwr_npu0", npu0_off),
	PMUCAL_PD_DESC(PD_NPU1, "blkpwr_npu1", npu1_off),
	PMUCAL_PD_DESC(PD_TNR, "blkpwr_tnr", tnr_off),
	PMUCAL_PD_DESC(PD_VRA, "blkpwr_vra", vra_off),
	PMUCAL_PD_DESC(PD_VTS, "blkpwr_vts", vts_off),
	PMUCAL_PD_DESC(PD_SSP, "blkpwr_ssp", ssp_off),
};
unsigned int pmucal_pd_list_size = ARRAY_SIZE(pmucal_pd_list);
