#include <linux/module.h>
#include <linux/wmi.h>
#include <linux/acpi.h>

#define GMWMI_GUID "644C5791-B7B0-4123-A90B-E93876E0DAAD"

struct smi_struct {
	__le16 a0, a1;
	__le32 a2, a3, a4, a5, a6, rev0, rev1;
} __packed;

static void do_query(struct wmi_device *wdev, const char *label)
{
	union acpi_object *out = wmidev_block_query(wdev, 0);
	if (!out) { pr_err("gmwmi: %s query failed\n", label); return; }
	if (out->type == ACPI_TYPE_BUFFER && out->buffer.length >= sizeof(struct smi_struct)) {
		struct smi_struct *r = (struct smi_struct *)out->buffer.pointer;
		pr_info("gmwmi: [%s] DisplayStatus=%u HDMIStatus=%u InitDisplayStatus=%u\n",
			label, le32_to_cpu(r->a2), le32_to_cpu(r->a3), le32_to_cpu(r->a4));
	}
	kfree(out);
}

static int gmwmi_probe(struct wmi_device *wdev, const void *context)
{
	struct smi_struct cmd = {0};
	struct acpi_buffer in = { sizeof(cmd), &cmd };

	do_query(wdev, "before");

	cmd.a0 = cpu_to_le16(64256); // set
	cmd.a1 = cpu_to_le16(515);
	cmd.a2 = cpu_to_le32(2);     // 2 = Nvidia, 1 = Intel

	if (ACPI_FAILURE(wmidev_block_set(wdev, 0, &in))) {
		pr_err("gmwmi: set failed\n");
		return 0;
	}
	pr_info("gmwmi: Nvidia module succeeded\n");

	do_query(wdev, "after");
	return 0;
}

static const struct wmi_device_id gmwmi_id_table[] = { { .guid_string = GMWMI_GUID }, {} };
MODULE_DEVICE_TABLE(wmi, gmwmi_id_table);
static struct wmi_driver gmwmi_driver = {
	.driver = { .name = "gmwmi_set_nvidia" },
	.id_table = gmwmi_id_table,
	.probe = gmwmi_probe,
};
module_wmi_driver(gmwmi_driver);
MODULE_LICENSE("MIT");
MODULE_DESCRIPTION("Set Nvidia GPU");
