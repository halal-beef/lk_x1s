
#ifndef __ACPM_MFD_H__
#define __ACPM_MFD_H__

/* Define shift */
#define SHIFT_BYTE			(8)
#define SHIFT_REG			(0)
#define SHIFT_TYPE			(8)
#define SHIFT_FUNC			(0)
#define SHIFT_DEST			(8)
#define SHIFT_CNT			(8)
#define SHIFT_WRITE_VAL			(8)
#define SHIFT_UPDATE_VAL		(8)
#define SHIFT_UPDATE_MASK		(16)
#define SHIFT_RETURN			(24)
#define SHIFT_BULK_VAL   		SHIFT_BYTE
#define SHIFT_CHANNEL			(12)

/* Define mask */
#define MASK_BYTE			(0xFF)
#define MASK_REG			MASK_BYTE
#define MASK_TYPE			(0xF)
#define MASK_FUNC       		MASK_BYTE
#define MASK_DEST			MASK_BYTE
#define MASK_CNT			MASK_BYTE
#define MASK_WRITE_VAL			MASK_BYTE
#define MASK_UPDATE_VAL			MASK_BYTE
#define MASK_UPDATE_MASK		MASK_BYTE
#define MASK_RETURN			MASK_BYTE
#define MASK_BULK_VAL 			MASK_BYTE
#define MASK_CHANNEL			(0xF)

/* Command */
#define set_protocol(data, protocol)	        	((data & MASK_##protocol) << SHIFT_##protocol)
#define read_protocol(data, protocol)	        	((data >> SHIFT_##protocol) & MASK_##protocol)

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

enum mfd_func {
        FUNC_READ,
        FUNC_WRITE,
        FUNC_UPDATE,
        FUNC_BULK_READ,
        FUNC_BULK_WRITE,
};

#endif /* __ACPM_MFD_H__ */
