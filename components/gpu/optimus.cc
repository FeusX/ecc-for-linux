#include "optimus.hh"

void OptimusSwitch::write_file(const char* path, const char* content, bool exec)
{
  FILE *f = fopen(path, "w");
  if(!f)
  { std::cout << "[ERROR] Could not write to the path: " << path << std::endl; return; }

  fputs(content, f);
  fclose(f);
  if(exec) { chmod(path, 0755); }
  std::cout << "[LOG] Written to the address: " << path << std::endl;
}

void OptimusSwitch::optimus_remove(const char* path)
{
  if(access(path, F_OK) == 0)
  {
    if(remove(path) == 0)
    { std::cout << "[LOG] Removed: " << path << std::endl; }
    else
    { std::cout << "[WARNING] Could not remove: " << path << std::endl; }
  }
}

void OptimusSwitch::edit_sddm(void)
{
  if(access(SDDM_XSETUP_BAK_PATH, F_OK) != 0) { return; }

  FILE *b = fopen(SDDM_XSETUP_BAK_PATH, "r");
  if(b)
  {
    char buf[8192];
    size_t n = fread(buf, 1, sizeof(buf) - 1, b);
    buf[n] = '\0';
    fclose(b);
    write_file(SDDM_XSETUP_PATH, buf, 1);
  }

  optimus_remove(SDDM_XSETUP_BAK_PATH);
}

void OptimusSwitch::cleanup(void)
{
  optimus_remove(BLACKLIST_PATH);
  optimus_remove(UDEV_INTEGRATED_PATH);
  optimus_remove(UDEV_PM_PATH);
  optimus_remove(XORG_PATH);
  optimus_remove(EXTRA_XORG_PATH);
  optimus_remove(MODESET_PATH);
  optimus_remove(LIGHTDM_SCRIPT_PATH);
  optimus_remove(LIGHTDM_CONFIG_PATH);
  optimus_remove(LEGACY_XORG_PATH);
  optimus_remove(LEGACY_UDEV_RM_PATH);
  optimus_remove(LEGACY_UDEV_PM_PATH);

  edit_sddm();
}

void OptimusSwitch::rebuild_initramfs(void)
{
  const char *cmd = NULL; 
  if(access("/etc/debian_version", F_OK) == 0)
  { cmd = "update-initramfs -u -k all"; }
  else if(access("/etc/arch-release", F_OK) == 0)
  { cmd = "mkinitcpio -P"; }

  if(!cmd)
  { std::cout << "[WARNING] Could not detect the distro, skipping initramfs rebuild..." << std::endl; return; }

  std::cout << "[LOG] Rebuilding initramfs..." << std::endl;
  if(system(cmd) != 0)
  { std::cout << "[ERROR] initramfs rebuild failed." << std::endl; }
}

static void pci_rescan(void)
{
  FILE *f = fopen("/sys/bus/pci/rescan", "w");
  if(f)
  {
    fputs("1", f);
    fclose(f);
  }
  else
  {
    std::cout << "[WARNING] Could not trigger PCI rescan." << std::endl;
  }
}

bool OptimusSwitch::get_nvidia_pci_bus(char *out, size_t outlen)
{
  pci_rescan();
  FILE *p = popen("lspci", "r");
  if(!p) { return false; }

  char line[512];
  char slot[32] = {0};
  bool found = false;

  while(fgets(line, sizeof(line), p))
  {
    if(strstr(line, "NVIDIA") &&
      (strstr(line, "VGA compatible controller") || strstr(line, "3D controller")))
    {
       sscanf(line, "%31s", slot);
       found = true;
       break;
    }
  }

  pclose(p);
  if(!found) { return false; }

  char *dom = strstr(slot, "0000:");
  if(dom) { memmove(dom, dom + 5, strlen(dom + 5) + 1); }

  int bus, dev, func;

  if(sscanf(slot, "%x:%x.%x", &bus, &dev, &func) != 3) { return false; }
  snprintf(out, outlen, "PCI:%d:%d:%d", bus, dev, func);

  return true;
}

char *OptimusSwitch::get_display_manager(void)
{
  FILE *f = fopen("/etc/systemd/system/display-manager.service", "r");
  if(!f) { return NULL; }

  static char dm[64] = {0};
  char line[256];
  char *result = NULL;

  while(fgets(line, sizeof(line), f))
  {
    char *pos = strstr(line, "ExecStart=");
    if(pos)
    {
      char *path = pos + strlen("ExecStart=");
      char *nl = strchr(path, '\n');
      if(nl) *nl = '\0';
      char *base = strrchr(path, '/');
      strncpy(dm, base ? base + 1 : path, sizeof(dm) - 1);
      result = dm;
      break;
    }
  }

  fclose(f);
  return result;
}

void OptimusSwitch::switch_to_igpu(void)
{
  std::system("systemctl disable nvidia-persistenced.service >/dev/null 2>&1");
  std::cout << "[OK] Disabled nvidia-persistenced.service" << std::endl;

  cleanup();

  write_file(BLACKLIST_PATH, BLACKLIST_CONTENT);
  write_file(UDEV_INTEGRATED_PATH, UDEV_INTEGRATED_CONTENT);

  rebuild_initramfs();

  std::cout << "[OK] Succesfully switched to iGPU. Reboot for changes to take effect." << std::endl;
}

void OptimusSwitch::switch_to_dgpu(void)
{
  std::system("systemctl enable nvidia-persistenced.service >/dev/null 2>&1");
  std::cout << "[OK] Enabled nvidia-persistenced.service" << std::endl;

  cleanup();

  char bus[32];
  if(!get_nvidia_pci_bus(bus, sizeof(bus)))
  { std::cout << "[ERROR] Could not find the NVIDIA GPU via lspci." << std::endl; return; }

  char xorg_content[2048];

  std::snprintf(xorg_content, sizeof(xorg_content), XORG_INTEL_TEMPLATE, bus);

  write_file(XORG_PATH, xorg_content);
  write_file(MODESET_PATH, MODESET_CONTENT);

  char *dm = get_display_manager();
  if(dm)
  {
    char xrandr_script[1024];
    snprintf(xrandr_script, sizeof(xrandr_script), XRANDR_SCRIPT_TEMPLATE, "modesetting");

    if(strcmp(dm, "sddm") == 0)
    {
      if(access(SDDM_XSETUP_PATH, F_OK) == 0)
      {
         FILE *cur = fopen(SDDM_XSETUP_PATH, "r");
         if(cur)
         {
           char buf[8192];
           size_t n = fread(buf, 1, sizeof(buf) - 1, cur);
           buf[n] = '\0';
           fclose(cur);
           write_file(SDDM_XSETUP_BAK_PATH, buf, 0);
           write_file(SDDM_XSETUP_PATH, xrandr_script, 1);
         }
      }
    }
    else if(strcmp(dm, "lightdm") == 0)                                                                                                    
    {                                                                                                                                      
      write_file(LIGHTDM_SCRIPT_PATH, xrandr_script, 1);                                                                                   
      write_file(LIGHTDM_CONFIG_PATH, LIGHTDM_CONFIG_CONTENT, 0);                                                                          
    } 
  }

  rebuild_initramfs();

  std::cout << "reboot it" << std::endl;
}
