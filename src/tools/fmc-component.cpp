/******************************************************************************
    COPYRIGHT (c) 2020 by Featuremine Corporation.
    This software has been provided pursuant to a License Agreement
    containing restrictions on its use.  This software contains
    valuable trade secrets and proprietary information of
    FeatureMine Corporation and is protected by law.  It may not be
    copied or distributed in any form or medium, disclosed to third
    parties, reverse engineered or used in any manner not provided
    for in said License Agreement except with the prior written
    authorization Featuremine Corporation.
*****************************************************************************/

#include <tclap/CmdLine.h>

#include <fmc/component.h>
#include <fmc/config.h>
#include <fmc/reactor.h>
#include <fmc/signals.h>

#include <fmc++/mpl.hpp>

#include <ytp/version.h>

#include <atomic>

static std::atomic<bool> run = true;
static void sig_handler(int s) { run = false; }

struct deleter_t {
  void operator()(struct fmc_component_module *ptr) {
    fmc_component_module_del(ptr);
  }
  void operator()(struct fmc_cfg_sect_item *ptr) { fmc_cfg_sect_del(ptr); }
};
struct initdestroy_t {
  void init(fmc_fd &fd, const char *path) {
    fmc_error_t *err;
    fd = fmc_fopen(path, READ, &err);
    fmc_runtime_error_unless(!err)
        << "Unable to open file " << path << ": " << fmc_error_msg(err);
  }
  void destroy(fmc_fd fd) {
    if (fd != -1) {
      fmc_error_t *err;
      fmc_fclose(fd, &err);
      fmc_runtime_error_unless(!err)
          << "Unable to close file " << fd << ": " << fmc_error_msg(err);
    }
  }

  void init(struct fmc_component_sys &sys) { fmc_component_sys_init(&sys); }
  void destroy(struct fmc_component_sys &sys) {
    fmc_component_sys_destroy(&sys);
  }
};
template <typename T, typename InitDestroy> struct scopevar_t {
  template <typename... Args> scopevar_t(Args &&... args) {
    InitDestroy().init(value, std::forward<Args>(args)...);
  }
  ~scopevar_t() { InitDestroy().destroy(value); }
  scopevar_t(const scopevar_t &) = delete;
  T value;
};
using module_ptr = fmc_component_module *;
using type_ptr = struct fmc_component_type *;
using config_ptr = std::unique_ptr<fmc_cfg_sect_item, deleter_t>;
using schema_ptr = struct fmc_cfg_node_spec *;
using sys_ptr = scopevar_t<fmc_component_sys, initdestroy_t>;
using file_ptr = scopevar_t<fmc_fd, initdestroy_t>;

int main(int argc, char **argv) {
  fmc_set_signal_handler(sig_handler);

  TCLAP::CmdLine cmd("FMC component loader", ' ', YTP_VERSION);

  TCLAP::ValueArg<std::string> mainArg(
      "s", "section", "Main section to be used to load the module", true,
      "main", "section");
  cmd.add(mainArg);

  TCLAP::ValueArg<std::string> cfgArg("c", "config", "Configuration path", true,
                                      "config.ini", "config_path");
  cmd.add(cfgArg);

  TCLAP::UnlabeledValueArg<std::string> moduleArg("module", "Module name", true,
                                                  "module", "module");
  cmd.add(moduleArg);

  TCLAP::UnlabeledValueArg<std::string> componentArg(
      "component", "Component name", true, "component", "component");
  cmd.add(componentArg);

  cmd.parse(argc, argv);

  sys_ptr sys;
  fmc_error_t *err;

  auto module = module_ptr(
      fmc_component_module_get(&sys.value, moduleArg.getValue().c_str(), &err));
  fmc_runtime_error_unless(!err)
      << "Unable to load module " << moduleArg.getValue() << ": "
      << fmc_error_msg(err);

  auto type = fmc_component_module_type_get(
      module, componentArg.getValue().c_str(), &err);
  fmc_runtime_error_unless(!err)
      << "Unable to get component type " << componentArg.getValue() << ": "
      << fmc_error_msg(err);

  config_ptr cfg;
  {
    schema_ptr schema = type->tp_cfgspec;
    file_ptr config_file(cfgArg.getValue().c_str());
    cfg = config_ptr(fmc_cfg_sect_parse_ini_file(
        schema, config_file.value, mainArg.getValue().c_str(), &err));
    fmc_runtime_error_unless(!err)
        << "Unable to load configuration file: " << fmc_error_msg(err);
  }

  fmc_component *component = fmc_component_new(type, cfg.get(), &err);
  fmc_runtime_error_unless(!err)
      << "Unable to load component " << componentArg.getValue() << ": "
      << fmc_error_msg(err);

  struct fmc_reactor r;
  fmc_reactor_init(&r);
  fmc_reactor_component_add(&r, component, &err);
  fmc_runtime_error_unless(!err)
      << "Unable to add component " << componentArg.getValue() << " to reactor : "
      << fmc_error_msg(err);

  fmc_reactor_run(&r);

  fmc_reactor_destroy(&r);
  return 0;
}
