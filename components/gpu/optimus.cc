#include "optimus.hh"

void OptimusSwitch::write_file(const char* path, const char* content, bool exec = false)
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

void OptimusSwitch::switch_to_igpu(void)
{
  std::system("systemctl disable nvidia-persistenced.service >/dev/null 2>&1");
  std::cout << "Disabled nvidia-persistenced.service" << std::endl;

  cleanup();

  write_file(BLACKLIST_PATH, BLACKLIST_CONTENT);
  write_file(UDEV_INTEGRATED_PATH, UDEV_INTEGRATED_CONTENT);

  rebuild_initramfs();

  std::cout << "[OK] Succesfully switched to iGPU. Reboot for changes to take effect." << std::endl;
}

void OptimusSwitch::switch_to_dgpu(void)
{

}
