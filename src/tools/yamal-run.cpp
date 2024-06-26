/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <tclap/CmdLine.h>

#include <fmc/component.h>
#include <fmc/config.h>
#include <fmc/process.h>
#include <fmc/reactor.h>
#include <fmc/signals.h>

#include <fmc++/mpl.hpp>
#include <fmc++/strings.hpp>

#include <ytp/version.h>
#include <ytp/yamal.h>

#include <json/json.hpp>
#include <set>
#include <unordered_map>
#include <vector>

#define JSON_PARSER_BUFF_SIZE 8192

struct fmc_reactor r;
static void sig_handler(int s) { fmc_reactor_stop(&r); }

struct deleter_t {
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

  try {

    TCLAP::CmdLine cmd("FMC component loader", ' ', YTP_VERSION);

    TCLAP::ValueArg<std::string> mainArg(
        "s", "section", "Main section to be used to load the module", false,
        "main", "section");
    cmd.add(mainArg);

    TCLAP::UnlabeledValueArg<std::string> cfgArg("configuration",
                                                 "Configuration path", false,
                                                 "config.ini", "config_path");
    cmd.add(cfgArg);

    TCLAP::ValueArg<std::string> backwardsCompatibilityCfgArg(
        "c", "config", "Configuration path", false, "config.ini",
        "config_path");
    cmd.add(backwardsCompatibilityCfgArg);

    TCLAP::ValueArg<std::string> moduleArg("m", "module", "Module name", false,
                                           "module", "module");
    cmd.add(moduleArg);

    TCLAP::ValueArg<std::string> componentArg(
        "o", "component", "Component name", false, "component", "component");
    cmd.add(componentArg);

    TCLAP::SwitchArg schedArg("k", "sched",
                              "Run component scheduled (live by default)");
    cmd.add(schedArg);

    TCLAP::ValueArg<int> affinityArg("a", "affinity",
                                     "set the CPU affinity of the main process",
                                     false, 0, "cpuid");
    cmd.add(affinityArg);

    TCLAP::ValueArg<int> priorityArg(
        "p", "priority", "set the priority of the main process (1-99)", false,
        1, "priority");
    cmd.add(priorityArg);

    TCLAP::ValueArg<int> auxArg("x", "auxiliary",
                                "set the CPU affinity of the auxiliary process",
                                false, 0, "cpuid");
    cmd.add(auxArg);

    TCLAP::SwitchArg jsonSwitch("j", "json", "Use JSON configuration.", false);
    cmd.add(jsonSwitch);

    cmd.parse(argc, argv);

    sys_ptr sys;
    fmc_error_t *err;

    fmc_runtime_error_unless((moduleArg.isSet() && componentArg.isSet()) ||
                             (!moduleArg.isSet() && !componentArg.isSet()))
        << "Invalid combination of arguments. module and component must either "
           "be provided through args or config.";

    fmc_runtime_error_unless(cfgArg.isSet() !=
                             backwardsCompatibilityCfgArg.isSet())
        << "Please provide the configuration only as either a labeled or "
           "unlabeled argument";

    const char *cfg_arg = cfgArg.isSet()
                              ? cfgArg.getValue().c_str()
                              : backwardsCompatibilityCfgArg.getValue().c_str();

    bool json_switch =
        jsonSwitch.getValue() || fmc::ends_with(cfg_arg, ".json");

    fmc_runtime_error_unless((json_switch && !mainArg.isSet()) ||
                             (!json_switch && mainArg.isSet()))
        << "Invalid combination of arguments. main section argument must be "
           "provided only when ini config is used.";

    fmc_reactor_init(&r);

    std::unordered_map<std::string, fmc_component *> components;

    fmc::scope_end_call destroy_reactor([&]() { fmc_reactor_destroy(&r); });

    auto read_file = [](const char *fpath, fmc_error_t **err) {
      fmc_error_clear(err);
      file_ptr config_file(fpath);
      std::vector<char> buffer;
      size_t cfgsz = 0;
      while (true) {
        buffer.resize(cfgsz + JSON_PARSER_BUFF_SIZE);
        auto sz = fmc_fread(config_file.value, &buffer[cfgsz],
                            JSON_PARSER_BUFF_SIZE, err);
        fmc_runtime_error_unless(!*err)
            << "Unable to read configuration file: " << fmc_error_msg(*err);
        if (sz == 0) {
          break;
        }
        cfgsz += sz;
      }
      buffer.resize(cfgsz);
      return buffer;
    };

    auto load_config = [&](config_ptr &cfg, struct fmc_cfg_node_spec *type,
                           const char *section) {
      fmc_error_t *err;
      if (json_switch) {
        auto buffer = read_file(cfg_arg, &err);
        if (section) {
          auto sub_buffer = nlohmann::json::parse(buffer)[section].dump();
          cfg = config_ptr(fmc_cfg_sect_parse_json(type, sub_buffer.data(),
                                                   sub_buffer.size(), &err));
        } else {
          cfg = config_ptr(fmc_cfg_sect_parse_json(type, buffer.data(),
                                                   buffer.size(), &err));
        }
      } else {
        file_ptr config_file(cfg_arg);
        cfg = config_ptr(fmc_cfg_sect_parse_ini_file(type, config_file.value,
                                                     section, &err));
      }
      fmc_runtime_error_unless(!err)
          << "Unable to load configuration file: " << fmc_error_msg(err);
    };

    auto gen_component = [&sys, &load_config](
                             const char *module_name,
                             const char *component_name, const char *section,
                             struct fmc_component_input *inps) {
      config_ptr cfg;
      fmc_error_t *err;
      module_ptr module(
          fmc_component_module_get(&sys.value, module_name, &err));
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
      components.emplace(
          componentArg.getValue().c_str(),
          gen_component(
              moduleArg.getValue().c_str(), componentArg.getValue().c_str(),
              mainArg.isSet() ? mainArg.getValue().c_str() : nullptr, nullptr));
    } else if (json_switch) {
      auto buffer = read_file(cfg_arg, &err);
      nlohmann::json j_obj =
          nlohmann::json::parse(std::string_view(buffer.data(), buffer.size()));

      std::set<std::string> component_stack;
      std::function<void(std::string, nlohmann::json &)> process_component =
          [&](std::string name, nlohmann::json &val) {
            if (components.find(name) != components.end()) {
              return;
            }
            fmc_runtime_error_unless(component_stack.find(name) ==
                                     component_stack.end())
                << "Unable to process provided configuration, cycle found "
                   "while "
                   "processing component "
                << name << ". Component graph must not contain any cycles.";
            component_stack.insert(name);
            auto modulename = val["module"].get<std::string>();
            auto type = val["component"].get<std::string>();
            auto inputs = val["inputs"];
            auto config = val["config"].dump();

            std::vector<struct fmc_component_input> inps;
            auto inpsit = val.find("inputs");
            if (inpsit != val.end()) {
              for (auto &inp : *inpsit) {
                auto inpcomponent = inp["component"].get<std::string>();

                if (components.find(inpcomponent.c_str()) == components.end()) {
                  auto it = j_obj.find(inpcomponent);
                  fmc_runtime_error_unless(it != j_obj.end())
                      << "Unable to find component " << inpcomponent
                      << " in components configuration.";
                  process_component(it.key(), it.value());
                }

                auto inpout_name_it = inp.find("name");
                auto inpindex_it = inp.find("index");

                fmc_runtime_error_unless(
                    (inpout_name_it != inp.end() && inpindex_it == inp.end()) ||
                    (inpout_name_it == inp.end() && inpindex_it != inp.end()))
                    << "Invalid combination of arguments for output of "
                       "component "
                    << inpcomponent << " please provide name or index.";

                if (inpout_name_it != inp.end()) {
                  size_t idx = fmc_component_out_idx(
                      components[inpcomponent.c_str()],
                      inpout_name_it->get<std::string>().c_str(), &err);
                  fmc_runtime_error_unless(!err)
                      << "Unable to obtain index of component: "
                      << fmc_error_msg(err);
                  inps.push_back(fmc_component_input{
                      components[inpcomponent.c_str()], idx});
                } else {
                  auto index = inpindex_it->get<int64_t>();
                  fmc_runtime_error_unless(
                      (index >= 0) ||
                      ((size_t)index <
                       fmc_component_out_sz(components[inpcomponent.c_str()])))
                      << "Index out of range for output of component " << name;
                  inps.push_back(fmc_component_input{
                      components[inpcomponent.c_str()], (size_t)index});
                }
              }
            }

            inps.push_back(fmc_component_input{nullptr, 0});

            config_ptr cfg;
            fmc_error_t *err;
            module_ptr module(
                fmc_component_module_get(&sys.value, modulename.c_str(), &err));
            fmc_runtime_error_unless(!err)
                << "Unable to load module " << modulename << ": "
                << fmc_error_msg(err);

            auto comptype =
                fmc_component_module_type_get(module, type.c_str(), &err);
            fmc_runtime_error_unless(!err)
                << "Unable to get component type " << type << ": "
                << fmc_error_msg(err);

            cfg = config_ptr(fmc_cfg_sect_parse_json(
                comptype->tp_cfgspec, config.data(), config.size(), &err));

            fmc_runtime_error_unless(!err)
                << "Unable to load configuration file: " << fmc_error_msg(err);

            fmc_component *component =
                fmc_component_new(&r, comptype, cfg.get(), &inps[0], &err);

            fmc_runtime_error_unless(!err)
                << "Unable to load component " << name << ": "
                << fmc_error_msg(err);

            components.emplace(name.c_str(), component);
            component_stack.erase(name);
          };

      for (auto it = j_obj.begin(); it != j_obj.end(); ++it) {
        process_component(it.key(), it.value());
      }

    } else {
      config_ptr cfg;

      load_config(cfg, yamal_run_spec,
                  mainArg.isSet() ? mainArg.getValue().c_str() : nullptr);

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
            auto component = fmc_cfg_sect_item_get(input_sect, "component");
            auto out_name = fmc_cfg_sect_item_get(input_sect, "name");
            auto index = fmc_cfg_sect_item_get(input_sect, "index");

            fmc_runtime_error_unless(
                components.find(component->node.value.str) != components.end())
                << "Unable to find component " << component->node.value.str
                << " component has not been created. Please reorder your "
                   "components appropriately.";

            fmc_runtime_error_unless((out_name && !index) ||
                                     (!out_name && index))
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
                  (index->node.value.int64 >= 0) ||
                  ((size_t)index->node.value.int64 <
                   fmc_component_out_sz(components[component->node.value.str])))
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

    if (auxArg.isSet()) {
      ytp_yamal_set_aux_thread_affinity(auxArg.getValue());
    }

    if (affinityArg.isSet()) {
      fmc_tid threadid = fmc_tid_cur(&err);
      fmc_runtime_error_unless(!err)
          << "Unable to get current thread id: " << fmc_error_msg(err);

      int cpuid = affinityArg.getValue();
      fmc_set_affinity(threadid, cpuid, &err);
      fmc_runtime_error_unless(!err)
          << "Unable to set current thread cpu affinity: "
          << fmc_error_msg(err);

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

  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  return 0;
}
