"""
        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

        """

"""
@package cli.py
@author Andrus Suvalau
@date 20 Jul 2020
@brief File contains test for yamal cli
"""

import jubilee
import extractor
from datetime import datetime, timedelta
import pytz
from sys import stdout
from featuremine.tools.protocol import OrderInfo, synchronize_position, proc_updates
from featuremine.tools.writer import TransportWriter
import os
from featuremine.tools.cli import YamalCLI
import yamal


def datetime_conv(date):
    return date - pytz.timezone("UTC").localize(datetime(1970, 1, 1))


def New_York_time(year, mon, day, h=0, m=0, s=0):
    return datetime_conv(pytz.timezone("America/New_York").
                         localize(datetime(year, mon, day, h, m, s)))


class Symbology:
    def __init__(self, symb_table):
        self.table = symb_table

    def info(self, hash):
        return self.table[hash]

    def imnts(self):
        return self.table.keys()


class Universe:
    def __init__(self, universe_keys):
        self.universe_keys = universe_keys

    def get(self, name):
        if name != "test":
            raise LookupError("unknown universe name")
        return self.universe_keys


class Strategy:
    def __init__(self, active_strategy, name, platform):
        self.venue = "VERY_LONG_VENUE_NAME"
        self.account = 101
        self.name = name
        self.platform = platform
        self.imnts = self.platform.universe.get("test")
        self.lognpos = True
        self.active_strategy = active_strategy

        self.handlers = {
            imnt: self.platform.handlers[name](imnt, self.venue, self.account)
            for imnt in self.imnts
        }

        self.platform.setup()

    def setup(self):

        now = self.platform.clock.now()

        self.platform.timer.schedule_once(now + timedelta(milliseconds=30), self.platform.shutdown)

        if not self.active_strategy:
            return

        wake_up_times = [
            now + timedelta(milliseconds=10),
            now + timedelta(milliseconds=20)
        ]

        for wake_up_time in wake_up_times:
            self.platform.timer.schedule_once(wake_up_time, self.timer_clbck)

    def timer_clbck(self):
        for imnt in self.imnts:
            order = self.handlers[imnt].place(px=100.0, qty=400, side=jubilee.trade_side.BID(
            ) if self.lognpos else jubilee.trade_side.ASK(), tif=jubilee.time_in_force.DAY())
            order = self.handlers[imnt].custom(
                qty=400,
                side=jubilee.trade_side.BID() if self.lognpos else jubilee.trade_side.ASK(),
                tif=jubilee.time_in_force.DAY(),
                marketable=True)
            self.lognpos = not self.lognpos


class FillModel(object):
    def __init__(self):
        self.type = 0

    def place(self, order):
        if self.type == 1:
            order.placed(px=99.94)
        elif self.type == 2:
            order.placed()
            order.filled(px=99.95)
        elif self.type == 3:
            order.placed()
            order.filled(qty=200, px=100.00)
        elif self.type == 4:
            order.placed()
            order.filled(qty=200, px=99.00)
        elif self.type == 5:
            order.failed()
        elif self.type == 6:
            order.placed()
            order.rejected()
        elif self.type == 7:
            self.type = 0

        self.type = self.type + 1
        return True

    def cancel(self, order):
        if not order.alive():
            order.cancel_rej()
        else:
            order.canceled()
        return True


class Platform:
    def __init__(self, suffix, symb_table, bod_table, active_strategy):
        self.trading_venue = "VERY_LONG_VENUE_NAME"
        self.config = {
            "extractor": {
                "license_file": "../test/extractor/test.lic"
            },
            "accounts": {
                "MAIN": {
                    "ids": [101]
                }
            },
            "venues": {
                "VERY_LONG_VENUE_NAME": {
                    "type": "jubilee::py_venue",
                    "accounts": {
                        "101": {
                            "id": "TEST"
                        }
                    },
                    "locate": "TEST",
                    "exdest": "NYSE",
                    "symbology": "NYSE",
                    "ids": {
                        "persist": False
                    },
                    "session": FillModel(),
                    "marking_log": "composite_log",
                    "validation_log": "composite_log",
                }
            },
            "strategies": {
                "simple_python": {
                    "type": "jubilee::py_strategy",
                    "order_closure": {}
                }
            },
            "reports": {
                "order_report": {
                    "type": "jubilee::report::order_log_report",
                    "id_type": "ticker",
                    "log_name": "composite_log",
                },
            },
            "logs": {
                "backends": [
                    {
                        "type": "jubilee::platform::logs::std_log",
                        "separator": " ",
                        "streams": ["composite_log"]
                    }
                ],
                "streams": {
                    "composite_log": [],
                },
                "use_cpu_affinity": False
            },
            "control": {
                "timeout": timedelta(seconds=2),
                "poll_callable": self.poll,
                "on_lost_keep_alive": self.lost_keep_alive,
                "poll_interval": timedelta(seconds=1)
            },
            "validation_params": {"id_type": "ticker"},
            "sampling": [
            ],
            "python_states": {
                "poll_interval": timedelta(seconds=1)
            },
            "transport": {
                "ids_file": "/tmp/ids_file.{0}".format(suffix),
                "data_file": "/tmp/data_file.{0}".format(suffix),
                "pub_name": "instance_name",
                "symbol": "ticker"
            },
            "currency": {
                "entity_currency": {
                    "MAIN": "USD"
                },
                "type": "jubilee::currency::constant_conversion",
                "symbology": "currency",
                "currencies": ["USD"],
                "conversions": {
                    "USD": 1.00
                }
            }
        }
        self.symbology = Symbology(symb_table)
        self.universe = Universe(symb_table.keys())
        self.bod_table = bod_table
        self.strategies_map = {}

        self.imnt_acc_order_info = None
        self.last_imnt_acc_order_info = None
        self.acc_order_info = None
        self.last_acc_order_info = None

        self.imnt_entity_order_info = None
        self.last_imnt_entity_order_info = None
        self.entity_order_info = None
        self.last_entity_order_info = None
        self.active_strategy = active_strategy

    def strategies(self, name):
        if name not in self.strategies_map:
            self.strategies_map[name] = Strategy(self.active_strategy, name=name, platform=self)
        return self.strategies_map[name]

    def build_features(self, graph):
        pass

    def remote_state_poll(self):
        pass

    def poll(self):
        return True

    def lost_keep_alive(self):
        self.shutdown()

    def setup(self):
        imnts = self.universe.get("test")
        account = 101
        entity = "MAIN"

        self.imnt_acc_order_info = {
            imnt: self.order_info(instrument=imnt, account=account)
            for imnt in imnts
        }

        self.last_imnt_acc_order_info = {
            imnt: {"open_count": 0, "open_share": 0, "open_notional": 0.0,
                   "desired_count": 0, "desired_share": 0, "desired_notional": 0.0,
                   "acked_count": 0, "acked_share": 0, "acked_notional": 0.0,
                   "filled_count": 0, "filled_share": 0, "filled_notional": 0.0}
            for imnt in imnts
        }

        self.acc_order_info = self.order_info(account=account)
        self.last_acc_order_info = {"open_count": 0, "open_share": 0, "open_notional": 0.0,
                                    "desired_count": 0, "desired_share": 0, "desired_notional": 0.0,
                                    "acked_count": 0, "acked_share": 0, "acked_notional": 0.0,
                                    "filled_count": 0, "filled_share": 0, "filled_notional": 0.0}

        self.imnt_entity_order_info = {
            imnt: self.order_info(instrument=imnt, entity=entity)
            for imnt in imnts
        }

        self.last_imnt_entity_order_info = {
            imnt: {"open_count": 0, "open_share": 0, "open_notional": 0.0,
                   "desired_count": 0, "desired_share": 0, "desired_notional": 0.0,
                   "acked_count": 0, "acked_share": 0, "acked_notional": 0.0,
                   "filled_count": 0, "filled_share": 0, "filled_notional": 0.0}
            for imnt in imnts
        }

        self.entity_order_info = self.order_info(entity=entity)
        self.last_entity_order_info = {"open_count": 0, "open_share": 0, "open_notional": 0.0,
                                       "desired_count": 0, "desired_share": 0, "desired_notional": 0.0,
                                       "acked_count": 0, "acked_share": 0, "acked_notional": 0.0,
                                       "filled_count": 0, "filled_share": 0, "filled_notional": 0.0}

    def on_start(self):
        for name, strategy in self.strategies_map.items():
            strategy.setup()

    def on_shutdown(self, err):
        imnts = self.universe.get("test")

        for imnt in imnts:
            self.last_imnt_acc_order_info[imnt]["open_count"] = self.imnt_acc_order_info[imnt].total_open_count()
            self.last_imnt_acc_order_info[imnt]["open_share"] = self.imnt_acc_order_info[imnt].total_open_share()
            self.last_imnt_acc_order_info[imnt]["open_notional"] = self.imnt_acc_order_info[imnt].total_open_notional()
            self.last_imnt_acc_order_info[imnt]["desired_count"] = self.imnt_acc_order_info[imnt].total_desired_count()
            self.last_imnt_acc_order_info[imnt]["desired_share"] = self.imnt_acc_order_info[imnt].total_desired_share()
            self.last_imnt_acc_order_info[imnt]["desired_notional"] = self.imnt_acc_order_info[imnt].total_desired_notional(
            )
            self.last_imnt_acc_order_info[imnt]["acked_count"] = self.imnt_acc_order_info[imnt].total_acked_count()
            self.last_imnt_acc_order_info[imnt]["acked_share"] = self.imnt_acc_order_info[imnt].total_acked_share()
            self.last_imnt_acc_order_info[imnt]["acked_notional"] = self.imnt_acc_order_info[imnt].total_acked_notional()
            self.last_imnt_acc_order_info[imnt]["filled_count"] = self.imnt_acc_order_info[imnt].total_filled_count()
            self.last_imnt_acc_order_info[imnt]["filled_share"] = self.imnt_acc_order_info[imnt].total_filled_share()
            self.last_imnt_acc_order_info[imnt]["filled_notional"] = self.imnt_acc_order_info[imnt].total_filled_notional(
            )

        self.last_acc_order_info["open_count"] = self.acc_order_info.total_open_count()
        self.last_acc_order_info["open_share"] = self.acc_order_info.total_open_share()
        self.last_acc_order_info["open_notional"] = self.acc_order_info.total_open_notional()
        self.last_acc_order_info["desired_count"] = self.acc_order_info.total_desired_count()
        self.last_acc_order_info["desired_share"] = self.acc_order_info.total_desired_share()
        self.last_acc_order_info["desired_notional"] = self.acc_order_info.total_desired_notional()
        self.last_acc_order_info["acked_count"] = self.acc_order_info.total_acked_count()
        self.last_acc_order_info["acked_share"] = self.acc_order_info.total_acked_share()
        self.last_acc_order_info["acked_notional"] = self.acc_order_info.total_acked_notional()
        self.last_acc_order_info["filled_count"] = self.acc_order_info.total_filled_count()
        self.last_acc_order_info["filled_share"] = self.acc_order_info.total_filled_share()
        self.last_acc_order_info["filled_notional"] = self.acc_order_info.total_filled_notional()

        for imnt in imnts:
            self.last_imnt_entity_order_info[imnt]["open_count"] = self.imnt_entity_order_info[imnt].total_open_count()
            self.last_imnt_entity_order_info[imnt]["open_share"] = self.imnt_entity_order_info[imnt].total_open_share()
            self.last_imnt_entity_order_info[imnt]["open_notional"] = self.imnt_entity_order_info[imnt].total_open_notional(
            )
            self.last_imnt_entity_order_info[imnt]["desired_count"] = self.imnt_entity_order_info[imnt].total_desired_count(
            )
            self.last_imnt_entity_order_info[imnt]["desired_share"] = self.imnt_entity_order_info[imnt].total_desired_share(
            )
            self.last_imnt_entity_order_info[imnt]["desired_notional"] = self.imnt_entity_order_info[imnt].total_desired_notional(
            )
            self.last_imnt_entity_order_info[imnt]["acked_count"] = self.imnt_entity_order_info[imnt].total_acked_count(
            )
            self.last_imnt_entity_order_info[imnt]["acked_share"] = self.imnt_entity_order_info[imnt].total_acked_share(
            )
            self.last_imnt_entity_order_info[imnt]["acked_notional"] = self.imnt_entity_order_info[imnt].total_acked_notional(
            )
            self.last_imnt_entity_order_info[imnt]["filled_count"] = self.imnt_entity_order_info[imnt].total_filled_count(
            )
            self.last_imnt_entity_order_info[imnt]["filled_share"] = self.imnt_entity_order_info[imnt].total_filled_share(
            )
            self.last_imnt_entity_order_info[imnt]["filled_notional"] = self.imnt_entity_order_info[imnt].total_filled_notional(
            )

        self.last_entity_order_info["open_count"] = self.entity_order_info.total_open_count()
        self.last_entity_order_info["open_share"] = self.entity_order_info.total_open_share()
        self.last_entity_order_info["open_notional"] = self.entity_order_info.total_open_notional()
        self.last_entity_order_info["desired_count"] = self.entity_order_info.total_desired_count()
        self.last_entity_order_info["desired_share"] = self.entity_order_info.total_desired_share()
        self.last_entity_order_info["desired_notional"] = self.entity_order_info.total_desired_notional()
        self.last_entity_order_info["acked_count"] = self.entity_order_info.total_acked_count()
        self.last_entity_order_info["acked_share"] = self.entity_order_info.total_acked_share()
        self.last_entity_order_info["acked_notional"] = self.entity_order_info.total_acked_notional()
        self.last_entity_order_info["filled_count"] = self.entity_order_info.total_filled_count()
        self.last_entity_order_info["filled_share"] = self.entity_order_info.total_filled_share()
        self.last_entity_order_info["filled_notional"] = self.entity_order_info.total_filled_notional()

    def bod(self, imnt, account):
        return self.bod_table[account][imnt]

    def remote_state(self, key):
        account_values = {
            "throttle_window": timedelta(seconds=1),
            "count_throttle": 1000000000,
            "share_throttle": 1000000000,
            "notional_throttle": 1000000000.0,
            "open_count_max": 1000000000,
            "open_share_max": 1000000000,
            "open_notional_max": 1000000000.0,
            "total_volume_max": 1000000000,
            "total_position_max": 1000000000,
            "total_notional_max": 1000000000.0,
            "restricted": False
        }
        imnt_values = {
            "throttle_window": timedelta(seconds=1),
            "count_throttle": 1000000000,
            "share_throttle": 1000000000,
            "notional_throttle": 1000000000.0,
            "open_count_max": 1000000000,
            "open_share_max": 1000000000,
            "open_notional_max": 1000000000.0,
            "total_volume_max": 1000000000,
            "total_position_max": 1000000000,
            "total_notional_max": 1000000000.0,
            "restricted": False,
            "locates": 1000000000,
            "ssr": False,
        }
        marking_values = {
            "offset": 0
        }

        prefix, _, suffix = key.partition("/")
        if prefix == "validation":
            prefix, _, suffix = suffix.partition("/")
            if prefix == "account":
                prefix, _, suffix = suffix.partition("/")
                return account_values[suffix]
            elif prefix == "instrument:account":
                prefix, _, suffix = suffix.partition("/")
                return imnt_values[suffix]
        elif prefix == "marking":
            prefix, _, suffix = suffix.partition("/")
            if prefix == "instrument:entity":
                prefix, _, suffix = suffix.partition("/")
                return marking_values[suffix]


if __name__ == "__main__":

    symb_table = {
        1: {"ticker": "A",
            "currency": "USD",
            "symbology": {
                "NYSE": {"symbol": "A", "suffix": ""}
            },
            "notional_factor": 1.0
            },
        2: {"ticker": "AA",
            "currency": "USD",
            "symbology": {
                "NYSE": {"symbol": "AA", "suffix": ""}
            },
            "notional_factor": 1.0
            },
        3: {"ticker": "BA",
            "currency": "USD",
            "symbology": {
                "NYSE": {"symbol": "BA", "suffix": ""}
            },
            "notional_factor": 1.0
            }
    }

    bod_table = {
        101: {k: {"position": 0, "cash": 0.0} for k in symb_table.keys()}
    }

    if os.path.exists("/tmp/ids_file.pos_sync"):
        os.remove("/tmp/ids_file.pos_sync")
    if os.path.exists("/tmp/data_file.pos_sync"):
        os.remove("/tmp/data_file.pos_sync")

    p = Platform('pos_sync', symb_table, bod_table, True)

    jubilee.live.run_platform(p)

    d = p.last_imnt_acc_order_info
    assert d[1]["open_count"] == 3
    assert d[1]["open_share"] == 1200
    assert d[1]["open_notional"] == 40000.0
    assert d[1]["desired_count"] == 3
    assert d[1]["desired_share"] == 1200
    assert d[1]["desired_notional"] == 40000.0
    assert d[1]["acked_count"] == 1
    assert d[1]["acked_share"] == 400
    assert d[1]["acked_notional"] == 0.0
    assert d[1]["filled_count"] == 0
    assert d[1]["filled_share"] == 0
    assert d[1]["filled_notional"] == 0.0
    assert d[2]["open_count"] == 2
    assert d[2]["open_share"] == 600
    assert d[2]["open_notional"] == 40000.0
    assert d[2]["desired_count"] == 2
    assert d[2]["desired_share"] == 600
    assert d[2]["desired_notional"] == 40000.0
    assert d[2]["acked_count"] == 2
    assert d[2]["acked_share"] == 600
    assert d[2]["acked_notional"] == 40000.0
    assert d[2]["filled_count"] == 3
    assert d[2]["filled_share"] == 1000
    assert d[2]["filled_notional"] == 99960.0
    assert d[3]["open_count"] == 3
    assert d[3]["open_share"] == 600
    assert d[3]["open_notional"] == 40000.0
    assert d[3]["desired_count"] == 3
    assert d[3]["desired_share"] == 600
    assert d[3]["desired_notional"] == 40000.0
    assert d[3]["acked_count"] == 3
    assert d[3]["acked_share"] == 600
    assert d[3]["acked_notional"] == 40000.0
    assert d[3]["filled_count"] == 3
    assert d[3]["filled_share"] == 600
    assert d[3]["filled_notional"] == 59600.0

    d = p.last_acc_order_info
    assert d["open_count"] == 8
    assert d["open_share"] == 2400
    assert d["open_notional"] == 120000.0
    assert d["desired_count"] == 8
    assert d["desired_share"] == 2400
    assert d["desired_notional"] == 120000.0
    assert d["acked_count"] == 6
    assert d["acked_share"] == 1600
    assert d["acked_notional"] == 80000.0
    assert d["filled_count"] == 6
    assert d["filled_share"] == 1600
    assert d["filled_notional"] == 159560.0

    d = p.last_imnt_entity_order_info
    assert d[1]["open_count"] == 3
    assert d[1]["open_share"] == 1200
    assert d[1]["open_notional"] == 40000.0
    assert d[1]["desired_count"] == 3
    assert d[1]["desired_share"] == 1200
    assert d[1]["desired_notional"] == 40000.0
    assert d[1]["acked_count"] == 1
    assert d[1]["acked_share"] == 400
    assert d[1]["acked_notional"] == 0.0
    assert d[1]["filled_count"] == 0
    assert d[1]["filled_share"] == 0
    assert d[1]["filled_notional"] == 0.0
    assert d[2]["open_count"] == 2
    assert d[2]["open_share"] == 600
    assert d[2]["open_notional"] == 40000.0
    assert d[2]["desired_count"] == 2
    assert d[2]["desired_share"] == 600
    assert d[2]["desired_notional"] == 40000.0
    assert d[2]["acked_count"] == 2
    assert d[2]["acked_share"] == 600
    assert d[2]["acked_notional"] == 40000.0
    assert d[2]["filled_count"] == 3
    assert d[2]["filled_share"] == 1000
    assert d[2]["filled_notional"] == 99960.0
    assert d[3]["open_count"] == 3
    assert d[3]["open_share"] == 600
    assert d[3]["open_notional"] == 40000.0
    assert d[3]["desired_count"] == 3
    assert d[3]["desired_share"] == 600
    assert d[3]["desired_notional"] == 40000.0
    assert d[3]["acked_count"] == 3
    assert d[3]["acked_share"] == 600
    assert d[3]["acked_notional"] == 40000.0
    assert d[3]["filled_count"] == 3
    assert d[3]["filled_share"] == 600
    assert d[3]["filled_notional"] == 59600.0

    d = p.last_entity_order_info
    assert d["open_count"] == 8
    assert d["open_share"] == 2400
    assert d["open_notional"] == 120000.0
    assert d["desired_count"] == 8
    assert d["desired_share"] == 2400
    assert d["desired_notional"] == 120000.0
    assert d["acked_count"] == 6
    assert d["acked_share"] == 1600
    assert d["acked_notional"] == 80000.0
    assert d["filled_count"] == 6
    assert d["filled_share"] == 1600
    assert d["filled_notional"] == 159560.0

    cli = YamalCLI("/tmp/ids_file.pos_sync", "/tmp/data_file.pos_sync")
    cli.onecmd("fill --pub_name instance_name --order_idx 1001 --quantity 200 --price 100.0 --filled_notional 20000.0")
    cli.onecmd("fill --pub_name instance_name --order_idx 1002 --quantity 400 --price 100.0 --filled_notional 40000.0")
    cli.onecmd("fill --pub_name instance_name --order_idx 1012 --quantity 100 --price 96.0 --filled_notional 9600.0")
    cli.onecmd("cancel --pub_name instance_name --order_idx 1001")
    cli.onecmd("cancel --pub_name instance_name --order_idx 1004")
    cli.onecmd("cancel --pub_name instance_name --order_idx 1005")
    cli.onecmd("cancel --pub_name instance_name --order_idx 1008")
    cli.onecmd("cancel --pub_name instance_name --order_idx 1009")
    cli.onecmd("cancel --pub_name instance_name --order_idx 1011")
    cli.onecmd("cancel --pub_name instance_name --order_idx 1012")

    def validate_platform():
        p = Platform('pos_sync', symb_table, bod_table, False)

        jubilee.live.run_platform(p)

        d = p.last_imnt_acc_order_info
        assert d[1]["open_count"] == 0
        assert d[1]["open_share"] == 0
        assert d[1]["open_notional"] == 0.0
        assert d[1]["desired_count"] == 0
        assert d[1]["desired_share"] == 0
        assert d[1]["desired_notional"] == 0.0
        assert d[1]["acked_count"] == 0
        assert d[1]["acked_share"] == 0
        assert d[1]["acked_notional"] == 0.0
        assert d[1]["filled_count"] == 2
        assert d[1]["filled_share"] == 600
        assert d[1]["filled_notional"] == 60000.0
        assert d[2]["open_count"] == 0
        assert d[2]["open_share"] == 0
        assert d[2]["open_notional"] == 0.0
        assert d[2]["desired_count"] == 0
        assert d[2]["desired_share"] == 0
        assert d[2]["desired_notional"] == 0.0
        assert d[2]["acked_count"] == 0
        assert d[2]["acked_share"] == 0
        assert d[2]["acked_notional"] == 0.0
        assert d[2]["filled_count"] == 3
        assert d[2]["filled_share"] == 1000
        assert d[2]["filled_notional"] == 99960.0
        assert d[3]["open_count"] == 0
        assert d[3]["open_share"] == 0
        assert d[3]["open_notional"] == 0.0
        assert d[3]["desired_count"] == 0
        assert d[3]["desired_share"] == 0
        assert d[3]["desired_notional"] == 0.0
        assert d[3]["acked_count"] == 0
        assert d[3]["acked_share"] == 0
        assert d[3]["acked_notional"] == 0.0
        assert d[3]["filled_count"] == 4
        assert d[3]["filled_share"] == 700
        assert d[3]["filled_notional"] == 69200.0

        d = p.last_acc_order_info
        assert d["open_count"] == 0
        assert d["open_share"] == 0
        assert d["open_notional"] == 0.0
        assert d["desired_count"] == 0
        assert d["desired_share"] == 0
        assert d["desired_notional"] == 0.0
        assert d["acked_count"] == 0
        assert d["acked_share"] == 0
        assert d["acked_notional"] == 0.0
        assert d["filled_count"] == 9
        assert d["filled_share"] == 2300
        assert d["filled_notional"] == 229160.0

        d = p.last_imnt_entity_order_info
        assert d[1]["open_count"] == 0
        assert d[1]["open_share"] == 0
        assert d[1]["open_notional"] == 0.0
        assert d[1]["desired_count"] == 0
        assert d[1]["desired_share"] == 0
        assert d[1]["desired_notional"] == 0.0
        assert d[1]["acked_count"] == 0
        assert d[1]["acked_share"] == 0
        assert d[1]["acked_notional"] == 0.0
        assert d[1]["filled_count"] == 2
        assert d[1]["filled_share"] == 600
        assert d[1]["filled_notional"] == 60000.0
        assert d[2]["open_count"] == 0
        assert d[2]["open_share"] == 0
        assert d[2]["open_notional"] == 0.0
        assert d[2]["desired_count"] == 0
        assert d[2]["desired_share"] == 0
        assert d[2]["desired_notional"] == 0.0
        assert d[2]["acked_count"] == 0
        assert d[2]["acked_share"] == 0
        assert d[2]["acked_notional"] == 0.0
        assert d[2]["filled_count"] == 3
        assert d[2]["filled_share"] == 1000
        assert d[2]["filled_notional"] == 99960.0
        assert d[3]["open_count"] == 0
        assert d[3]["open_share"] == 0
        assert d[3]["open_notional"] == 0.0
        assert d[3]["desired_count"] == 0
        assert d[3]["desired_share"] == 0
        assert d[3]["desired_notional"] == 0.0
        assert d[3]["acked_count"] == 0
        assert d[3]["acked_share"] == 0
        assert d[3]["acked_notional"] == 0.0
        assert d[3]["filled_count"] == 4
        assert d[3]["filled_share"] == 700
        assert d[3]["filled_notional"] == 69200.0

        d = p.last_entity_order_info
        assert d["open_count"] == 0
        assert d["open_share"] == 0
        assert d["open_notional"] == 0.0
        assert d["desired_count"] == 0
        assert d["desired_share"] == 0
        assert d["desired_notional"] == 0.0
        assert d["acked_count"] == 0
        assert d["acked_share"] == 0
        assert d["acked_notional"] == 0.0
        assert d["filled_count"] == 9
        assert d["filled_share"] == 2300
        assert d["filled_notional"] == 229160.0

    validate_platform()

    cli.onecmd("cancel_all --pub_name instance_name")

    validate_platform()

    t = TransportWriter("/tmp/ids_file.pos_sync", "/tmp/data_file.pos_sync", "instance_name")

    order = OrderInfo(1013)

    upd_arr = order.place_ack(0, 0, "USD", 400, 100.00, 40000.0, 0, "AA", "NYSE", 101, "MAIN")

    print(repr(order))
    print(str(order))
    print(order)

    proc_updates(t, upd_arr)

    p = Platform('pos_sync', symb_table, bod_table, False)

    jubilee.live.run_platform(p)

    d = p.last_imnt_acc_order_info
    assert d[1]["open_count"] == 0
    assert d[1]["open_share"] == 0
    assert d[1]["open_notional"] == 0.0
    assert d[1]["desired_count"] == 0
    assert d[1]["desired_share"] == 0
    assert d[1]["desired_notional"] == 0.0
    assert d[1]["acked_count"] == 0
    assert d[1]["acked_share"] == 0
    assert d[1]["acked_notional"] == 0.0
    assert d[1]["filled_count"] == 2
    assert d[1]["filled_share"] == 600
    assert d[1]["filled_notional"] == 60000.0
    assert d[2]["open_count"] == 1
    assert d[2]["open_share"] == 400
    assert d[2]["open_notional"] == 40000.0
    assert d[2]["desired_count"] == 1
    assert d[2]["desired_share"] == 400
    assert d[2]["desired_notional"] == 40000.0
    assert d[2]["acked_count"] == 1
    assert d[2]["acked_share"] == 400
    assert d[2]["acked_notional"] == 40000.0
    assert d[2]["filled_count"] == 3
    assert d[2]["filled_share"] == 1000
    assert d[2]["filled_notional"] == 99960.0
    assert d[3]["open_count"] == 0
    assert d[3]["open_share"] == 0
    assert d[3]["open_notional"] == 0.0
    assert d[3]["desired_count"] == 0
    assert d[3]["desired_share"] == 0
    assert d[3]["desired_notional"] == 0.0
    assert d[3]["acked_count"] == 0
    assert d[3]["acked_share"] == 0
    assert d[3]["acked_notional"] == 0.0
    assert d[3]["filled_count"] == 4
    assert d[3]["filled_share"] == 700
    assert d[3]["filled_notional"] == 69200.0

    d = p.last_acc_order_info
    assert d["open_count"] == 1
    assert d["open_share"] == 400
    assert d["open_notional"] == 40000.0
    assert d["desired_count"] == 1
    assert d["desired_share"] == 400
    assert d["desired_notional"] == 40000.0
    assert d["acked_count"] == 1
    assert d["acked_share"] == 400
    assert d["acked_notional"] == 40000.0
    assert d["filled_count"] == 9
    assert d["filled_share"] == 2300
    assert d["filled_notional"] == 229160.0

    d = p.last_imnt_entity_order_info
    assert d[1]["open_count"] == 0
    assert d[1]["open_share"] == 0
    assert d[1]["open_notional"] == 0.0
    assert d[1]["desired_count"] == 0
    assert d[1]["desired_share"] == 0
    assert d[1]["desired_notional"] == 0.0
    assert d[1]["acked_count"] == 0
    assert d[1]["acked_share"] == 0
    assert d[1]["acked_notional"] == 0.0
    assert d[1]["filled_count"] == 2
    assert d[1]["filled_share"] == 600
    assert d[1]["filled_notional"] == 60000.0
    assert d[2]["open_count"] == 1
    assert d[2]["open_share"] == 400
    assert d[2]["open_notional"] == 40000.0
    assert d[2]["desired_count"] == 1
    assert d[2]["desired_share"] == 400
    assert d[2]["desired_notional"] == 40000.0
    assert d[2]["acked_count"] == 1
    assert d[2]["acked_share"] == 400
    assert d[2]["acked_notional"] == 40000.0
    assert d[2]["filled_count"] == 3
    assert d[2]["filled_share"] == 1000
    assert d[2]["filled_notional"] == 99960.0
    assert d[3]["open_count"] == 0
    assert d[3]["open_share"] == 0
    assert d[3]["open_notional"] == 0.0
    assert d[3]["desired_count"] == 0
    assert d[3]["desired_share"] == 0
    assert d[3]["desired_notional"] == 0.0
    assert d[3]["acked_count"] == 0
    assert d[3]["acked_share"] == 0
    assert d[3]["acked_notional"] == 0.0
    assert d[3]["filled_count"] == 4
    assert d[3]["filled_share"] == 700
    assert d[3]["filled_notional"] == 69200.0

    d = p.last_entity_order_info
    assert d["open_count"] == 1
    assert d["open_share"] == 400
    assert d["open_notional"] == 40000.0
    assert d["desired_count"] == 1
    assert d["desired_share"] == 400
    assert d["desired_notional"] == 40000.0
    assert d["acked_count"] == 1
    assert d["acked_share"] == 400
    assert d["acked_notional"] == 40000.0
    assert d["filled_count"] == 9
    assert d["filled_share"] == 2300
    assert d["filled_notional"] == 229160.0

    upd_arr = order.fill(200, 98.00, 19600)

    proc_updates(t, upd_arr)

    p = Platform('pos_sync', symb_table, bod_table, False)

    jubilee.live.run_platform(p)

    d = p.last_imnt_acc_order_info
    assert d[1]["open_count"] == 0
    assert d[1]["open_share"] == 0
    assert d[1]["open_notional"] == 0.0
    assert d[1]["desired_count"] == 0
    assert d[1]["desired_share"] == 0
    assert d[1]["desired_notional"] == 0.0
    assert d[1]["acked_count"] == 0
    assert d[1]["acked_share"] == 0
    assert d[1]["acked_notional"] == 0.0
    assert d[1]["filled_count"] == 2
    assert d[1]["filled_share"] == 600
    assert d[1]["filled_notional"] == 60000.0
    assert d[2]["open_count"] == 1
    assert d[2]["open_share"] == 200
    assert d[2]["open_notional"] == 20000.0
    assert d[2]["desired_count"] == 1
    assert d[2]["desired_share"] == 200
    assert d[2]["desired_notional"] == 20000.0
    assert d[2]["acked_count"] == 1
    assert d[2]["acked_share"] == 200
    assert d[2]["acked_notional"] == 20000.0
    assert d[2]["filled_count"] == 4
    assert d[2]["filled_share"] == 1200
    assert d[2]["filled_notional"] == 119560.0
    assert d[3]["open_count"] == 0
    assert d[3]["open_share"] == 0
    assert d[3]["open_notional"] == 0.0
    assert d[3]["desired_count"] == 0
    assert d[3]["desired_share"] == 0
    assert d[3]["desired_notional"] == 0.0
    assert d[3]["acked_count"] == 0
    assert d[3]["acked_share"] == 0
    assert d[3]["acked_notional"] == 0.0
    assert d[3]["filled_count"] == 4
    assert d[3]["filled_share"] == 700
    assert d[3]["filled_notional"] == 69200.0

    d = p.last_acc_order_info
    assert d["open_count"] == 1
    assert d["open_share"] == 200
    assert d["open_notional"] == 20000.0
    assert d["desired_count"] == 1
    assert d["desired_share"] == 200
    assert d["desired_notional"] == 20000.0
    assert d["acked_count"] == 1
    assert d["acked_share"] == 200
    assert d["acked_notional"] == 20000.0
    assert d["filled_count"] == 10
    assert d["filled_share"] == 2500
    assert d["filled_notional"] == 248760.0

    d = p.last_imnt_entity_order_info
    assert d[1]["open_count"] == 0
    assert d[1]["open_share"] == 0
    assert d[1]["open_notional"] == 0.0
    assert d[1]["desired_count"] == 0
    assert d[1]["desired_share"] == 0
    assert d[1]["desired_notional"] == 0.0
    assert d[1]["acked_count"] == 0
    assert d[1]["acked_share"] == 0
    assert d[1]["acked_notional"] == 0.0
    assert d[1]["filled_count"] == 2
    assert d[1]["filled_share"] == 600
    assert d[1]["filled_notional"] == 60000.0
    assert d[2]["open_count"] == 1
    assert d[2]["open_share"] == 200
    assert d[2]["open_notional"] == 20000.0
    assert d[2]["desired_count"] == 1
    assert d[2]["desired_share"] == 200
    assert d[2]["desired_notional"] == 20000.0
    assert d[2]["acked_count"] == 1
    assert d[2]["acked_share"] == 200
    assert d[2]["acked_notional"] == 20000.0
    assert d[2]["filled_count"] == 4
    assert d[2]["filled_share"] == 1200
    assert d[2]["filled_notional"] == 119560.0
    assert d[3]["open_count"] == 0
    assert d[3]["open_share"] == 0
    assert d[3]["open_notional"] == 0.0
    assert d[3]["desired_count"] == 0
    assert d[3]["desired_share"] == 0
    assert d[3]["desired_notional"] == 0.0
    assert d[3]["acked_count"] == 0
    assert d[3]["acked_share"] == 0
    assert d[3]["acked_notional"] == 0.0
    assert d[3]["filled_count"] == 4
    assert d[3]["filled_share"] == 700
    assert d[3]["filled_notional"] == 69200.0

    d = p.last_entity_order_info
    assert d["open_count"] == 1
    assert d["open_share"] == 200
    assert d["open_notional"] == 20000.0
    assert d["desired_count"] == 1
    assert d["desired_share"] == 200
    assert d["desired_notional"] == 20000.0
    assert d["acked_count"] == 1
    assert d["acked_share"] == 200
    assert d["acked_notional"] == 20000.0
    assert d["filled_count"] == 10
    assert d["filled_share"] == 2500
    assert d["filled_notional"] == 248760.0

    cli.onecmd("cancel_all --pub_name instance_name")

    p = Platform('pos_sync', symb_table, bod_table, False)

    jubilee.live.run_platform(p)

    d = p.last_imnt_acc_order_info
    assert d[1]["open_count"] == 0
    assert d[1]["open_share"] == 0
    assert d[1]["open_notional"] == 0.0
    assert d[1]["desired_count"] == 0
    assert d[1]["desired_share"] == 0
    assert d[1]["desired_notional"] == 0.0
    assert d[1]["acked_count"] == 0
    assert d[1]["acked_share"] == 0
    assert d[1]["acked_notional"] == 0.0
    assert d[1]["filled_count"] == 2
    assert d[1]["filled_share"] == 600
    assert d[1]["filled_notional"] == 60000.0
    assert d[2]["open_count"] == 0
    assert d[2]["open_share"] == 0
    assert d[2]["open_notional"] == 0.0
    assert d[2]["desired_count"] == 0
    assert d[2]["desired_share"] == 0
    assert d[2]["desired_notional"] == 0.0
    assert d[2]["acked_count"] == 0
    assert d[2]["acked_share"] == 0
    assert d[2]["acked_notional"] == 0.0
    assert d[2]["filled_count"] == 4
    assert d[2]["filled_share"] == 1200
    assert d[2]["filled_notional"] == 119560.0
    assert d[3]["open_count"] == 0
    assert d[3]["open_share"] == 0
    assert d[3]["open_notional"] == 0.0
    assert d[3]["desired_count"] == 0
    assert d[3]["desired_share"] == 0
    assert d[3]["desired_notional"] == 0.0
    assert d[3]["acked_count"] == 0
    assert d[3]["acked_share"] == 0
    assert d[3]["acked_notional"] == 0.0
    assert d[3]["filled_count"] == 4
    assert d[3]["filled_share"] == 700
    assert d[3]["filled_notional"] == 69200.0

    d = p.last_acc_order_info
    assert d["open_count"] == 0
    assert d["open_share"] == 0
    assert d["open_notional"] == 0.0
    assert d["desired_count"] == 0
    assert d["desired_share"] == 0
    assert d["desired_notional"] == 0.0
    assert d["acked_count"] == 0
    assert d["acked_share"] == 0
    assert d["acked_notional"] == 0.0
    assert d["filled_count"] == 10
    assert d["filled_share"] == 2500
    assert d["filled_notional"] == 248760.0

    d = p.last_imnt_entity_order_info
    assert d[1]["open_count"] == 0
    assert d[1]["open_share"] == 0
    assert d[1]["open_notional"] == 0.0
    assert d[1]["desired_count"] == 0
    assert d[1]["desired_share"] == 0
    assert d[1]["desired_notional"] == 0.0
    assert d[1]["acked_count"] == 0
    assert d[1]["acked_share"] == 0
    assert d[1]["acked_notional"] == 0.0
    assert d[1]["filled_count"] == 2
    assert d[1]["filled_share"] == 600
    assert d[1]["filled_notional"] == 60000.0
    assert d[2]["open_count"] == 0
    assert d[2]["open_share"] == 0
    assert d[2]["open_notional"] == 0.0
    assert d[2]["desired_count"] == 0
    assert d[2]["desired_share"] == 0
    assert d[2]["desired_notional"] == 0.0
    assert d[2]["acked_count"] == 0
    assert d[2]["acked_share"] == 0
    assert d[2]["acked_notional"] == 0.0
    assert d[2]["filled_count"] == 4
    assert d[2]["filled_share"] == 1200
    assert d[2]["filled_notional"] == 119560.0
    assert d[3]["open_count"] == 0
    assert d[3]["open_share"] == 0
    assert d[3]["open_notional"] == 0.0
    assert d[3]["desired_count"] == 0
    assert d[3]["desired_share"] == 0
    assert d[3]["desired_notional"] == 0.0
    assert d[3]["acked_count"] == 0
    assert d[3]["acked_share"] == 0
    assert d[3]["acked_notional"] == 0.0
    assert d[3]["filled_count"] == 4
    assert d[3]["filled_share"] == 700
    assert d[3]["filled_notional"] == 69200.0

    d = p.last_entity_order_info
    assert d["open_count"] == 0
    assert d["open_share"] == 0
    assert d["open_notional"] == 0.0
    assert d["desired_count"] == 0
    assert d["desired_share"] == 0
    assert d["desired_notional"] == 0.0
    assert d["acked_count"] == 0
    assert d["acked_share"] == 0
    assert d["acked_notional"] == 0.0
    assert d["filled_count"] == 10
    assert d["filled_share"] == 2500
    assert d["filled_notional"] == 248760.0
