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

#include <unordered_map>
#include <vector>

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
  template <typename... Args> scopevar_t(Args &&...args) {
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

/*
[main]
components=component1sec,component2sec

[component1cfg]

[component1sec]
name="component1"
module="mymodule"
type="componenttype"
config=component1cfg

[component2cfg]
...

[component2sec]
name="component2"
module="mymodule"
type="componenttype"
inputs=[component1out1, component1out2]
config=component2cfg

[component1out1]
component=component1
name=outputone

[component1out2]
component=component1
index=0
*/

fmc_cfg_node_spec input_spec[] = {
    fmc_cfg_node_spec{
        .key = "component",
        .descr = "Component name to be used as input",
        .required = true,
        .type =
            fmc_cfg_type{
                .type = FMC_CFG_STR,
            },
    },
    fmc_cfg_node_spec{
        .key = "name",
        .descr = "Component output name to be used as input",
        .required = false,
        .type =
            fmc_cfg_type{
                .type = FMC_CFG_STR,
            },
    },
    fmc_cfg_node_spec{
        .key = "index",
        .descr = "Component output index to be used as input",
        .required = false,
        .type =
            fmc_cfg_type{
                .type = FMC_CFG_INT64,
            },
    },
    {NULL}};

struct fmc_cfg_type input_description_spec = {.type = FMC_CFG_SECT,
                                              .spec = {.node = input_spec}};

fmc_cfg_node_spec component_cfg_spec[] = {
    fmc_cfg_node_spec{
        .key = "module",
        .descr = "Module name",
        .required = true,
        .type =
            fmc_cfg_type{
                .type = FMC_CFG_STR,
            },
    },
    fmc_cfg_node_spec{
        .key = "type",
        .descr = "Component type name",
        .required = true,
        .type =
            fmc_cfg_type{
                .type = FMC_CFG_STR,
            },
    },
    fmc_cfg_node_spec{
        .key = "inputs",
        .descr = "Input descriptions",
        .required = false,
        .type =
            fmc_cfg_type{
                .type = FMC_CFG_ARR,
                .spec{
                    .array = &input_description_spec,
                },
            },
    },
    fmc_cfg_node_spec{
        .key = "config",
        .descr = "Component configuration section name",
        .required = true,
        .type =
            fmc_cfg_type{
                .type = FMC_CFG_STR,
            },
    },
    fmc_cfg_node_spec{
        .key = "name",
        .descr = "Component configuration section name",
        .required = true,
        .type =
            fmc_cfg_type{
                .type = FMC_CFG_STR,
            },
    },
    {NULL}};

struct fmc_cfg_type component_description_spec = {
    .type = FMC_CFG_SECT, .spec = {.node = component_cfg_spec}};

fmc_cfg_node_spec yamal_run_spec[] = {
    fmc_cfg_node_spec{
        .key = "components",
        .descr = "Array of individual component configurations",
        .required = true,
        .type =
            fmc_cfg_type{
                .type = FMC_CFG_ARR,
                .spec{
                    .array = &component_description_spec,
                },
            },
    },
    {NULL}};

int main(int argc, char **argv) {
  TCLAP::CmdLine cmd("FMC component loader", ' ', YTP_VERSION);

  TCLAP::ValueArg<std::string> mainArg(
      "s", "section", "Main section to be used to load the module", true,
      "main", "section");
  cmd.add(mainArg);

  TCLAP::ValueArg<std::string> cfgArg("c", "config", "Configuration path", true,
                                      "config.ini", "config_path");
  cmd.add(cfgArg);

  TCLAP::ValueArg<std::string> moduleArg("m", "module", "Module name", false,
                                         "module", "module");
  cmd.add(moduleArg);

  TCLAP::ValueArg<std::string> componentArg("o", "component", "Component name",
                                            false, "component", "component");
  cmd.add(componentArg);

  TCLAP::SwitchArg schedArg("k", "sched",
                            "Run component scheduled (live by default)");
  cmd.add(schedArg);

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

  fmc_runtime_error_unless((moduleArg.isSet() && componentArg.isSet()) ||
                           (!moduleArg.isSet() && !componentArg.isSet()))
      << "Invalid combination of arguments. module and component must either "
         "be provided through args or config.";

  fmc_reactor_init(&r);

  std::unordered_map<std::string, fmc_component *> components;

  fmc::scope_end_call destroy_reactor([&]() {
    for (auto &&[name, comp] : components) {
      fmc_component_del(comp);
    }
    fmc_reactor_destroy(&r);
  });

  auto load_config = [&cfgArg](config_ptr &cfg, struct fmc_cfg_node_spec *type,
                               const char *section) {
    file_ptr config_file(cfgArg.getValue().c_str());
    fmc_error_t *err;
    cfg = config_ptr(
        fmc_cfg_sect_parse_ini_file(type, config_file.value, section, &err));
    fmc_runtime_error_unless(!err)
        << "Unable to load configuration file: " << fmc_error_msg(err);
  };

  auto gen_component = [&sys, &load_config](const char *module_name,
                                            const char *component_name,
                                            const char *section,
                                            struct fmc_component_input *inps) {
    config_ptr cfg;
    fmc_error_t *err;
    module_ptr module(fmc_component_module_get(&sys.value, module_name, &err));
    fmc_runtime_error_unless(!err) << "Unable to load module " << module_name
                                   << ": " << fmc_error_msg(err);

    auto type = fmc_component_module_type_get(module, component_name, &err);
    fmc_runtime_error_unless(!err)
        << "Unable to get component type " << component_name << ": "
        << fmc_error_msg(err);

    load_config(cfg, type->tp_cfgspec, section);

    fmc_component *component =
        fmc_component_new(&r, type, cfg.get(), inps, &err);

    fmc_runtime_error_unless(!err)
        << "Unable to load component " << component_name << ": "
        << fmc_error_msg(err);

    return component;
  };

  if (moduleArg.isSet() && componentArg.isSet()) {
    components.emplace(componentArg.getValue().c_str(),
                       gen_component(moduleArg.getValue().c_str(),
                                     componentArg.getValue().c_str(),
                                     mainArg.getValue().c_str(), nullptr));
  } else {
    config_ptr cfg;

    load_config(cfg, yamal_run_spec, mainArg.getValue().c_str());

    auto arr = fmc_cfg_sect_item_get(cfg.get(), "components");
    for (auto elem = arr->node.value.arr; elem; elem = elem->next) {
      auto module = fmc_cfg_sect_item_get(elem->item.value.sect, "module");
      auto type = fmc_cfg_sect_item_get(elem->item.value.sect, "type");
      auto inputs = fmc_cfg_sect_item_get(elem->item.value.sect, "inputs");
      auto config = fmc_cfg_sect_item_get(elem->item.value.sect, "config");
      auto name = fmc_cfg_sect_item_get(elem->item.value.sect, "name");

      std::vector<struct fmc_component_input> inps;
      if (inputs) {
        for (auto inp = inputs->node.value.arr; inp; inp = inp->next) {
          auto input_sect = inp->item.value.sect;
          auto component =
              fmc_cfg_sect_item_get(inp->item.value.sect, "component");
          auto out_name = fmc_cfg_sect_item_get(inp->item.value.sect, "name");
          auto index = fmc_cfg_sect_item_get(inp->item.value.sect, "index");

          fmc_runtime_error_unless(components.find(component->node.value.str) !=
                                   components.end())
              << "Unable to find component " << component->node.value.str
              << " component has not been created. Please reorder your "
                 "components appropriately.";

          fmc_runtime_error_unless((out_name && !index) || (!out_name && index))
              << "Invalid combination of arguments for output of component "
              << out_name << " please provide name or index.";

          if (out_name) {
            size_t idx =
                fmc_component_out_idx(components[component->node.value.str],
                                      out_name->node.value.str, &err);
            fmc_runtime_error_unless(!err)
                << "Unable to obtain index of component: "
                << fmc_error_msg(err);
            inps.push_back(fmc_component_input{
                components[component->node.value.str], idx});
          } else {
            fmc_runtime_error_unless(
                index->node.value.int64 <
                fmc_component_out_sz(components[component->node.value.str]))
                << "Index out of range for output of component " << name;
            inps.push_back(
                fmc_component_input{components[component->node.value.str],
                                    (size_t)index->node.value.int64});
          }
        }
      }
      inps.push_back(fmc_component_input{nullptr, 0});

      components.emplace(name->node.value.str,
                         gen_component(module->node.value.str,
                                       type->node.value.str,
                                       config->node.value.str, &inps[0]));
    }
  }

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
  fmc_reactor_run(&r, !schedArg.getValue(), &err);
  fmc_runtime_error_unless(!err)
      << "Unable to run reactor : " << fmc_error_msg(err);

  return 0;
}
