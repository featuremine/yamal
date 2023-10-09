"""
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
"""

import threading
import yamal.sys_base as base
from typing import Optional, Callable, Any, List, Union
import weakref
import sys as system
if system.platform != "darwin":
    from os import sched_param, sched_setaffinity, sched_setscheduler, SCHED_FIFO
import json
from operator import getitem
from .yamal8 import *
from yamal.ytp import __version__ as __version__

class keydefaultdict(dict):
    def __init__(self, default_factory : Callable[[Any], Any]) -> None:
        self.default_factory = default_factory
        super().__init__()

    def __missing__(self, key : Any) -> Any:
        ret = self[key] = self.default_factory(key)
        return ret


class _sys(base.sys):
    @property
    def path(self) -> List[str]:
        return self.get_paths()

    @path.setter
    def path(self, value : List[str]) -> None:
        self.set_paths(value)


sys = _sys()
sys.path += system.path

class reactor(base.reactor):
    def run(self, live: bool = True, affinity : Optional[int] = None, priority : Optional[int] = None) -> None:
        checksignals = threading.current_thread() is threading.main_thread()
        if system.platform != "darwin":
            if affinity is not None:
                sched_setaffinity(0, (affinity,))
                if priority is not None:
                    p = sched_param(priority)
                    sched_setscheduler(0, SCHED_FIFO, p)
        else:
            if affinity is not None:
                raise RuntimeError("CPU Affinity cannot be set in Mac OS")
            if priority is not None:
                raise RuntimeError("Process scheduling priority cannot be set in Mac OS")
        super().run(live=live, checksignals=checksignals)

    def run_once(self, now : int) -> bool:
        return super().run_once(now=now)

    def stop(self) -> None:
        super().stop()

    def sched(self) -> int:
        return super().sched()

    def deploy(self, config: dict) -> None:
        components: dict = {}

        def gen_component(modname, compname, inputs, cfg):
            module = getattr(modules, modname)
            component_type = getattr(module, compname)
            return component_type(self, *inputs, **cfg)

        # Config example
        # {
        #     "coinbase_l2_producer": {
        #         "module": "bulldozer",
        #         "component": "coinbase_l2_producer",
        #         "config" : {
        #             "ws_host" : "ws-feed.exchange.coinbase.com",
        #             "universe" : ["ETH-BTC","BTC-USD","ADA-USD"]
        #         }
        #     },
        #     "ytp_consumer": {
        #         "module": "bulldozer",
        #         "component": "ytp_consumer",
        #         "inputs": [{"component":"coinbase_l2_producer","name":"coinbase_l2"}],
        #         "config": {
        #             "ytp_path" : "raw.ytp",
        #             "peer" : "feed_handler",
        #             "channel" : "coinbase_l2_raw"
        #         }
        #     }
        # }

        for component_name, component_cfg in config.items():
            inputs = []
            for input in component_cfg["inputs"] if "inputs" in component_cfg else []:
                if "name" in input:
                    inputs.append(getattr(components[input["component"]], input["name"]))
                else:
                    inputs.append(getitem(components[input["component"]], input["index"]))
            components[component_name] = gen_component(component_cfg["module"], component_cfg["component"],
                                                       inputs, component_cfg["config"])

class component:
    pass

class component_output:
    def __init__(self, comp: Any, idx: int) -> None:
        self.comp = comp
        self.idx = idx

class components:
    def __init__(self, modulename : str, module : int) -> None:
        def load_component(name : str) -> type:
            typename = f'yamal.modules.{modulename}.{name}'

            component_type = sys.get_component_type(module=module, name=name)

            arg_list = sys.get_component_type_spec(component_type=component_type)

            arg_names = ', '.join(map(lambda x: x[0], arg_list))

            def get_component(elem : component) -> base.component:
                return super(type(elem), elem).__getattribute__("_component")

            def _pre_init(self : component, react: reactor, inputs : List[Union[component_output, component]], cfg : dict) -> None:
                for key, argtype, req in arg_list:
                    if key in cfg and cfg[key] is None and argtype != 'NoneType':
                        del cfg[key]
                input_tuples = [(get_component(elem.comp), elem.idx) if isinstance(elem, component_output) else (get_component(elem), 0) for elem in inputs]
                super(type(self), self).__init__()
                super(type(self), self).__setattr__("_reactor", weakref.ref(react))
                comp = base.component(reactor=react, component_type=component_type, inputs=input_tuples, config=cfg)
                super(type(self), self).__setattr__("_component", comp)

            __init__ = eval(f'lambda cls, *args, **kwargs: _pre_init(cls, react=args[0], inputs=[*args[1:]], cfg=kwargs)', {
                '_pre_init': _pre_init
            })
            __init__.__name__ = '__init__'
            __init__.__doc__ = f'Initializes an instance of {typename}({arg_names})'
            __init__.__annotations__ = {}

            for key, argtype, req in arg_list:
                __init__.__annotations__[key] = eval(argtype if req else f'Optional[{argtype}]', {
                    'NoneType': type(None), 'Optional': Optional
                })
            __init__.__defaults__ = tuple(None for _ in arg_list)

            def _get_by_name(self: component, key: str) -> component_output:
                try:
                    if super(type(self), self).__getattribute__("_reactor")() is None:
                        raise RuntimeError("reactor used to create component has been deleted")
                    return component_output(self, get_component(self).out_idx(key))
                except RuntimeError as e:
                    if key in ['_component', '_reactor']:
                        raise e
                    return object.__getattribute__(self, key)

            def _get_by_id(self: component, key: int) -> component_output:
                if super(type(self), self).__getattribute__("_reactor")() is None:
                    raise RuntimeError("reactor used to create component has been deleted")
                if key >= get_component(self).out_sz():
                    raise RuntimeError("invalid index")
                return component_output(self, key)

            class_namespace = {
                '__doc__': f'{typename} class',
                '__init__': __init__,
                '__getattribute__': _get_by_name,
                '__getitem__': _get_by_id
            }

            return type(typename, (component, ), class_namespace)

        self._components = keydefaultdict(load_component)
        self._module = module

    @property
    def __path__(self) -> str:
        return sys.get_module_filepath(module=self._module)

    def __getattr__(self, name : str) -> Any:
        return self._components[name]


class Modules:
    def __init__(self) -> None:
        self._modules = keydefaultdict(lambda name: components(name, sys.get_module(name=name)))

    def __getattr__(self, name : str) -> Any:
        return self._modules[name]


modules = Modules()
