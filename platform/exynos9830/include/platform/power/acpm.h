/**
 * struct build_info
 */
#define APSHARE_BUILDINFO_OFFSET				(24 * 4)
#define BUILDINFO_ELEMENT_SIZE					(25)
struct build_info {
	char build_version[BUILDINFO_ELEMENT_SIZE];
	char build_time[BUILDINFO_ELEMENT_SIZE];
};

/**
 * struct plugin_ops - plugin callbacks to be provided to framework
 *
 * @ipc_handler:	handler to be executed when ipc for this plugin is arrived.
 * @irq_handler:	handler to be executed when hw irq for this plugin is arrived.
 * @timer_event_handler:handler to be executed when requested timer is expired.
 */
struct plugin_ops {
	u32 ipc_handler;
	u32 irq_handler;
	u32 timer_event_handler;
	u32 extern_func;
	struct build_info info;
};

/**
 * struct timer_desc - A descriptor for timer request
 *
 * @period:		requested period that framework executes timer_event_handler.
 * @multiplier:
 */
struct timer_desc {
	u32 period;
	u32 multiplier;
};

/**
 * struct plugin - The basic plugin structure
 *
 * @id:			Predefined id for this plugin.
 * @base_addr:		Predefined base addr for this plugin. (entrypoint)
 * @acpm_ops:		Framework callbacks.
 * @plugin_ops:		Plugin callbacks.
 * @timer:		Timer descriptor for this plugin.
 * @is_attached:	For dynamic plugin support.
 * @size:		The size of this plugin.
 */
struct plugin {
	u32 id;
	u32 base_addr;
	u32 acpm_ops;
	u32 plugin_ops;
	u32 secure_func_mask;
	u32 extern_func_mask;
	struct timer_desc timer;
	u8 is_attached;
	u32 size;
	u8 stay_attached;
	u32 fw_name;
};

#define get_acpm_plugin_element(__struct, __element)						\
	EXYNOS_ACPM_BASE + __struct##_address +					\
			((unsigned int *)&(__struct->__element) - (unsigned int *)__struct) * 4
