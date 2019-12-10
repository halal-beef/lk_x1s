#include <target/pmic.h>
#include <platform/sfr.h>

void target_early_init(void)
{
	if (*(unsigned int *)DRAM_BASE == 0xabcdef)
	{
		pmic_init();
		read_pmic_info();
		s2mu106_charger_init();
		fg_init_s2mu106();
	}
	return;
}

void target_init(void)
{
}

void target_init_for_usb(void)
{
}
