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
@author Andres Rangel
@date 18 May 2020
@brief File contains CLI module
"""

from .commands import *
import cmd

loc = locals()


class YtpCLI(cmd.Cmd):
    ''' YTP CLI '''
    intro = 'Featuremine command line utility.   Type help or ? to list commands.\n'
    prompt = 'ytp '

    def __init__(self, transact_file: str):
        super().__init__()
        self.transact_file = transact_file

    def onecmd(self, cmd: str) -> bool:
        global loc
        cmd = cmd.strip()
        if cmd.lower() == "exit":
            return True
        cmd_l = cmd.split(" ")
        cmd_args = cmd_l[1:]
        l = len(cmd_args)
        if l % 2 != 0:
            print("Failed to parse args")
            return False
        it = iter(cmd_args)
        args = {}
        for i in range(l // 2):
            key = next(it)
            val = next(it)
            if not key.startswith("--"):
                print("Invalid argument:", key)
                return False
            args[key.lstrip("-")] = val
        try:
            print(loc[cmd_l[0]](self.transact_file, **args))
            return True
        except BaseException as e:
            print("Error parsing command:", e)
        return False

    def emptyline(self) -> bool:
        pass
