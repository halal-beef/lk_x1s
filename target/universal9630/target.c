#include <stdio.h>
#include <target/pmic.h>
#include <dev/lk_acpm_ipc.h>
#include <platform/sfr.h>

void target_early_init(void)
{
	unsigned char reg;
	if (*(unsigned int *)DRAM_BASE == 0xabcdef) {
		pmic_init();
		read_pmic_info();
		s2mu107_sc_init();
		fg_init_s2mu107();

		/* UFS manually always-on */
		reg = 0xEC;
		i3c_write(0, 1, 0x46, reg);
		reg = 0;
		i3c_read(0, 1, 0x46, &reg);
		printf("LDO 17 set! value : %08x\n", reg);

		reg = 0xEE;
		i3c_write(0, 1, 0x47, reg);
		reg = 0;
		i3c_read(0, 1, 0x47, &reg);
		printf("LDO 18 set! value : %08x\n", reg);
	}
	return;
}

void target_init(void)
{
}

void target_init_for_usb(void)
{
}
