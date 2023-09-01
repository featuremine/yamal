"""
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
"""

import unittest
import os
import tempfile
import yamal
from yamal import modules, reactor, component_output

def get_tempfile_name(some_id='ttt'):
    return os.path.join(tempfile.gettempdir(), next(tempfile._get_candidate_names()) + "_" + some_id)

testoutput = os.path.join(tempfile.gettempdir(), "testoutput.txt")
yamal.sys.path += yamal.__path__

class TestLoadModule(unittest.TestCase):
    def test_load_module_success(self):
        testcomp = modules.testmodule.testcomponent

        try:
            os.remove(testoutput)
        except OSError:
            pass

        r = reactor()
        comp = testcomp(
            r,
            filename=testoutput,
            none=None,
            int64=3,
            boolean=True,
            float64=4.5,
            sect={"int64": 2},
            arr=[])

        self.assertTrue(comp.out1 is not None)
        self.assertTrue(isinstance(comp._component, component_output))
        self.assertTrue(isinstance(comp._reactor, component_output))
        with self.assertRaisesRegex(AttributeError, "'yamal.modules.testmodule.testcomponent' object has no attribute 'invalidout'") as cm:
            v = comp.invalidout

        self.assertTrue(comp[0] is not None)
        self.assertTrue(comp[1] is not None)
        self.assertTrue(comp[2] is not None)
        with self.assertRaisesRegex(RuntimeError, 'invalid index') as cm:
            v = comp[3]

        consumercomp = modules.testmodule.consumercomponent

        comp2 = consumercomp(
            r,
            comp)

        with self.assertRaisesRegex(RuntimeError, 'unable to find output with name _component in component') as cm:
            v = comp2._component
        with self.assertRaisesRegex(RuntimeError, 'unable to find output with name _reactor in component') as cm:
            v = comp2._reactor

        #multiple inputs
        consumercomp(
            r,
            comp,
            comp.out1,
            comp[0])

        r.run(live=False)

        r = None
        comp = None

        with open(testoutput) as f:
            self.assertEqual(f.readline(), "0\n")
            self.assertEqual(f.readline(), "1\n")
            self.assertEqual(f.readline(), "2\n")
            self.assertEqual(f.readline(), "3\n")
            self.assertEqual(f.readline(), "4\n")
            self.assertEqual(f.readline(), "")

    def test_load_module_notfound(self):
        with self.assertRaisesRegex(RuntimeError, 'component module testmodule2 was not found') as cm:
            testcomp = modules.testmodule2.testcomponent
            r = reactor()
            comp = testcomp(
                r,
                none=None,
                int64=3,
                boolean=True,
                float64=4.5,
                filename=testoutput,
                sect={"int64": 2},
                arr=[])

    def test_load_module_invalid_config(self):
        testcomp = modules.testmodule.testcomponent
        with self.assertRaisesRegex(RuntimeError, 'config error: missing required field int64') as cm:
            r = reactor()
            comp = testcomp(
                r,
                none=None,
                #int64=3,
                boolean=True,
                float64=4.5,
                filename=testoutput,
                sect={"int64": 2},
                arr=[])

    def test_load_module_optional_field(self):
        testcomp = modules.testmodule.testcomponent
        r = reactor()
        comp = testcomp(
            r,
            none=None,
            int64=3,
            boolean=True,
            #float64=4.5,
            filename=testoutput,
            sect={"int64": 2},
            arr=[])

    def test_comp_invalid_reactor(self):
        testcomp = modules.testmodule.testcomponent
        with self.assertRaisesRegex(RuntimeError, 'invalid reactor type') as cm:
            comp = testcomp(
                modules.testmodule,
                none=None,
                int64=3,
                boolean=True,
                float64=4.5,
                filename=testoutput,
                sect={"int64": 2},
                arr=[])

    def test_reactor_deploy(self):
        cfg = {
            "component_one" : {
                "module":"testmodule",
                "component":"testcomponent",
                "config": {
                    "filename":testoutput,
                    "none":None,
                    "int64":3,
                    "boolean":True,
                    "float64":4.5,
                    "sect":{"int64": 2},
                    "arr":[]
                }
            },
            "component_two" : {
                "module":"testmodule",
                "component":"consumercomponent",
                "config": {
                },
                "inputs": [
                    {
                        "component": "component_one",
                        "name": "out1"
                    },
                    {
                        "component": "component_one",
                        "index": 0
                    }
                ]
            }
        }

        r = reactor()

        r.deploy(cfg)

        r.run(live=False)

        r = None

        with open(testoutput) as f:
            self.assertEqual(f.readline(), "0\n")
            self.assertEqual(f.readline(), "1\n")
            self.assertEqual(f.readline(), "2\n")
            self.assertEqual(f.readline(), "3\n")
            self.assertEqual(f.readline(), "4\n")
            self.assertEqual(f.readline(), "")

if __name__ == '__main__':
    unittest.main()
