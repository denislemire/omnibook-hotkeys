/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/sysctl.h>
#include <sys/systm.h>

#include <contrib/dev/acpica/include/acpi.h>

static ACPI_HANDLE ercd;
static struct sysctl_ctx_list ctx;

static int
kbdlight_percent(int *percent)
{
	/* Same six-byte request as EC0._Q41, the HP illumination hotkey. */
	UINT8 request[] = { 0x45, 0x60, 0, 0, 0, 0 };
	ACPI_OBJECT argument = { .Buffer = {
	    .Type = ACPI_TYPE_BUFFER, .Length = sizeof(request),
	    .Pointer = request } };
	ACPI_OBJECT_LIST args = { .Count = 1, .Pointer = &argument };
	ACPI_BUFFER result = { ACPI_ALLOCATE_BUFFER, NULL };
	ACPI_OBJECT *obj;
	int error = EIO;

	if (ACPI_FAILURE(AcpiEvaluateObject(ercd, NULL, &args, &result)))
		return (EIO);
	obj = result.Pointer;
	if (obj != NULL && obj->Type == ACPI_TYPE_BUFFER &&
	    obj->Buffer.Length == 8) {
		int value = obj->Buffer.Pointer[0];
		if (value == 0 || value == 50 || value == 100) {
			*percent = value;
			error = 0;
		}
	}
	AcpiOsFree(result.Pointer);
	return (error);
}

static int
kbdlight_sysctl(SYSCTL_HANDLER_ARGS)
{
	int value, error;

	error = kbdlight_percent(&value);
	if (error != 0)
		return (error);
	return (sysctl_handle_int(oidp, &value, 0, req));
}

static int
kbdlight_event(module_t mod, int event, void *arg)
{
	char *sku;
	int value;
	bool match;

	switch (event) {
	case MOD_LOAD:
		sku = kern_getenv("smbios.system.sku");
		match = sku != NULL && strcmp(sku, "B8MX7UA#ABL") == 0;
		if (sku != NULL)
			freeenv(sku);
		if (!match || ACPI_FAILURE(AcpiGetHandle(NULL,
		    "\\_SB.PC00.LPCB.EC0.ERCD", &ercd)))
			return (ENXIO);
		if (kbdlight_percent(&value) != 0)
			return (EIO);
		sysctl_ctx_init(&ctx);
		if (SYSCTL_ADD_PROC(&ctx, SYSCTL_STATIC_CHILDREN(_hw),
		    OID_AUTO, "omnibook_kbdlight_percent",
		    CTLTYPE_INT | CTLFLAG_RD | CTLFLAG_MPSAFE, NULL, 0,
		    kbdlight_sysctl, "I", "Firmware keyboard illumination level (%)")
		    == NULL)
			return (ENOMEM);
		return (0);
	case MOD_UNLOAD:
		return (sysctl_ctx_free(&ctx));
	default:
		return (EOPNOTSUPP);
	}
}

static moduledata_t kbdlight_mod = {
	"omnibook_kbdlight", kbdlight_event, NULL
};
DECLARE_MODULE(omnibook_kbdlight, kbdlight_mod, SI_SUB_DRIVERS, SI_ORDER_ANY);
MODULE_DEPEND(omnibook_kbdlight, acpi, 1, 1, 1);
MODULE_VERSION(omnibook_kbdlight, 1);
