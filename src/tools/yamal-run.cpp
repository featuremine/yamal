/******************************************************************************

        COPYRIGHT (c) 2022 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

*****************************************************************************/

#include <tclap/CmdLine.h>

#include <fmc/component.h>
#include <fmc/config.h>
#include <fmc/process.h>
#include <fmc/reactor.h>
#include <fmc/signals.h>

#include <fmc++/mpl.hpp>

#include <ytp/version.h>


struct fmc_reactor r;
static void sig_handler(int s) { fmc_reactor_stop(&r); }

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

  void init(struct fmc_component_sys &sys) {
    fmc_component_sys_init(&sys);
    fmc_error_t *err;
    fmc_component_sys_paths_set_default(&sys, &err);
    fmc_runtime_error_unless(!err)
        << "Unable to set default search paths for component modules: "
        << fmc_error_msg(err);
  }
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

  TCLAP::SwitchArg liveArg(
      "l", "live", "Run component live (scheduled otherwise)");
  cmd.add(liveArg);

  TCLAP::ValueArg<int> affinityArg("a", "affinity",
                                   "set the CPU affinity of the main process",
                                   false, 0, "cpuid");
  cmd.add(affinityArg);

  TCLAP::ValueArg<int> priorityArg(
      "x", "priority", "set the priority of the main process (1-99)", false, 1,
      "priority");
  cmd.add(priorityArg);

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

  fmc_reactor_init(&r);
  fmc_component *component = fmc_component_new(&r, type, cfg.get(), nullptr, &err);
  fmc_runtime_error_unless(!err)
      << "Unable to load component " << componentArg.getValue() << ": "
      << fmc_error_msg(err);

  if (affinityArg.isSet()) {
    fmc_tid threadid = fmc_tid_cur(&err);
    fmc_runtime_error_unless(!err)
        << "Unable to get current thread id: " << fmc_error_msg(err);

    int cpuid = affinityArg.getValue();
    fmc_set_affinity(threadid, cpuid, &err);
    fmc_runtime_error_unless(!err)
        << "Unable to set current thread cpu affinity: " << fmc_error_msg(err);

    if (priorityArg.isSet()) {
      int priority = priorityArg.getValue();
      fmc_set_sched_fifo(threadid, priority, &err);
      fmc_runtime_error_unless(!err)
          << "Unable to set current thread priority: " << fmc_error_msg(err);
    }
  }

  fmc_set_signal_handler(sig_handler);
  fmc_reactor_run(&r, liveArg.getValue(), &err);
  fmc_runtime_error_unless(!err)
      << "Unable to run reactor : " << fmc_error_msg(err);

  fmc_reactor_destroy(&r);
  return 0;
}
