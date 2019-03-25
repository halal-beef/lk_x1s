#ifndef __PMUCAL_COMMON_H__
#define __PMUCAL_COMMON_H__

/* represents the value in access_type column in guide */
enum pmucal_seq_acctype {
	PMUCAL_READ = 0,
	PMUCAL_WRITE,
	PMUCAL_COND_READ,
	PMUCAL_COND_WRITE,
	PMUCAL_SAVE_RESTORE,
	PMUCAL_COND_SAVE_RESTORE,
	PMUCAL_WAIT,
	PMUCAL_WAIT_TWO,
	PMUCAL_CHECK_SKIP,
	PMUCAL_COND_CHECK_SKIP,
	PMUCAL_WRITE_WAIT,
	PMUCAL_WRITE_RETRY,
	PMUCAL_WRITE_RETRY_INV,
	PMUCAL_WRITE_RETURN,
	PMUCAL_SET_BIT_ATOMIC,
	PMUCAL_CLR_BIT_ATOMIC,
	PMUCAL_DELAY,
	PMUCAL_CLEAR_PEND,
};

/* represents each row in the PMU sequence guide */
struct pmucal_seq {
	u32 access_type;
	const char *sfr_name;
	u32 base_pa;
	u32 mask;
	u32 value;
};

#define PMUCAL_SEQ_DESC(_access_type, _sfr_name, _base_pa, _offset,	\
			_mask, _value, _cond_base_pa, _cond_offset,	\
			_cond_mask, _cond_value) {			\
	.access_type = _access_type,					\
	.sfr_name = _sfr_name,						\
	.base_pa = _base_pa | _offset,						\
	.mask = _mask,							\
	.value = _value,						\
}

#define PMUCAL_PREFIX			"PMUCAL:  "

#endif
